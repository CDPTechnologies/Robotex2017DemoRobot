var ANGULAR_CDP_STUDIOAPI_URL = window.location.host;

(function (angular) {

  // Create all modules and define dependencies to make sure they exist
  // and are loaded in the correct order to satisfy dependency injection
  // before all nested files are concatenated by Gulp

  // Config
  angular.module('angularCdp.config', [])
      .value('angularCdp.config', {
          debug: true
      });

  // Modules
  angular.module('angularCdp.directives', []);
  angular.module('angularCdp.services', []);
  angular.module('angularCdp',
      [
          'angularCdp.config',
          'angularCdp.cdp-bar',
          'angularCdp.cdp-button',
          'angularCdp.cdp-chart',
          'angularCdp.cdp-gauge',
          'angularCdp.cdp-input',
          'angularCdp.cdp-label'
      ]).
      service('studioapi', function() {
        var client = new studio.api.Client(ANGULAR_CDP_STUDIOAPI_URL);

        this.getClient = function() {
          return client;
        };
  });

})(angular);

var app = angular.module("angularCdp.cdp-button", []);

app.directive('cdpButton', ['studioapi', function(studioapi) {

    function link(scope, element, attrs) {

        var vm = scope;

        studioapi.getClient().find(attrs.routing).then(
            function (node) {
                vm.node = node;
            }
        );

        var onButtonClick = function () {
            vm.node.setValue(true);
        };

        element.on('click', onButtonClick);

        scope.$on('$destroy', function () {
            element.off('click', onButtonClick);
        });
    }

    return {
        restrict: 'E',
        replace : true,
        scope: {
            routing: '@'
        },
        transclude: true,
        template: '<button ng-transclude></button>',
        link: link
    };
}]);

var app = angular.module("angularCdp.cdp-bar", []);

app.directive('cdpBar', ['studioapi', function (studioapi) {

    function link(scope, element, attrs) {

        var vm = scope;
        this.subscription = {};

        studioapi.getClient().find(attrs.routing).then(
            function (node) {
                vm.subscription = {
                    node: node, callback: function (value) {
                        element.progress('set progress', value);
                    }
                };
                node.subscribeToValues(vm.subscription.callback);
            }
        );

        element.on('$destroy', function () {
            var s = vm.subscription;
            s.node.unsubscribeFromChildValues(s.callback);
        });

        element.progress(scope.$eval(attrs.options));
    }

    return {
        restrict: 'E',
        replace : true,
        transclude: true,
        scope: {
            routing: '@',
            options: '@'
        },
        template: '\
            <div">\
                <div class="bar"><div class="progress"></div></div>\
                <div class="label" ng-transclude=""></div>\
            </div>',
        link: link
    };
}]);

angular.module('angularCdp.cdp-gauge', [])
    .component('cdpGauge', {
        transclude: true,
        bindings: {
            width: '@',
            routing: '@',
            options: '@',
            minValue: '@',
            maxValue: '@',
            displayTextField: '='
        },
        controllerAs: 'vm',
        controller: ['$scope', 'studioapi', '$element', function ($scope, studioapi, $element) {

            var vm = this;
            vm.subscriptions = [];

            function createGauge() {
                var opts = {
                    angle: -0.2, // The span of the gauge arc
                    lineWidth: 0.2, // The line thickness
                    radiusScale: 1, // Relative radius
                    pointer: {
                        length: 0.6, // // Relative to gauge radius
                        strokeWidth: 0.035, // The thickness
                        color: '#000000' // Fill color
                    },
                    limitMax: false,     // If false, the max value of the gauge will be updated if value surpass max
                    limitMin: false,     // If true, the min value of the gauge will be fixed unless you set it manually
                    colorStart: '#6F6EA0',   // Colors
                    colorStop: '#C0C0DB',    // just experiment with them
                    strokeColor: '#EEEEEE',  // to see which ones work best for you
                    generateGradient: true,
                    highDpiSupport: true     // High resolution support
                };
                var userOptions = $scope.$eval(vm.options);
                opts = angular.merge(opts, userOptions);

                var target = $element.find('canvas')[0];
                var gauge = new Gauge(target).setOptions(opts);
                var textField = $element.find('div').find('div')[0];
                gauge.setTextField(textField);
                if (vm.maxValue)
                    gauge.maxValue = vm.maxValue;
                if (vm.minValue)
                    gauge.setMinValue(vm.minValue);
                gauge.animationSpeed = 1;
                //gauge.set(vm.minValue || 0);
                gauge.set(0);

                studioapi.getClient().find(vm.routing).then(
                    function (node) {
                        vm.subscription = {node: node, callback: function (value) {
                            gauge.set(value);
                        }};
                        node.subscribeToValues(vm.subscription.callback);
                    }
                );
            }

            this.$postLink = createGauge;

            this.$onDestroy = function () {
                var s = vm.subscription;
                s.node.unsubscribeFromChildValues(s.callback);
                console.log("Unsubscribed " + JSON.stringify(s.node));
            };

        }],
        template: '\
          <div style="display: inline-block;" ng-style="{ width: vm.width }"> \
            <canvas id="{{::$id}}" style="width: 100%"></canvas> \
            <div ng-show="vm.displayTextField !== false" id="{{::$id + \'Text\'}}" \
                 style="font-size: 15pt; text-align: center; top: -25px; "></div> \
          </div>'
    });

angular.module('angularCdp.cdp-chart', [])
    .component('cdpChart', {
        transclude: true,
        bindings: {
            width: '@',
            height: '@',
            options: '@',
            displayLegend: '='
        },
        controllerAs: 'vm',
        controller: ['$scope', 'studioapi', '$element', '$window',
            function ($scope, studioapi, $element, $window) {

            var vm = this;
            vm.subscriptions = [];
            vm.nodeNames = [];
            var chart;
            var options = {
                interpolation: 'step',
                grid: {fillStyle: '#f9f9f9'},
                labels: {fillStyle: '#2c1919', fontSize:12},
                timestampFormatter: SmoothieChart.timeFormatter,
                yRangeFunction: yRangeFunction,
                tooltip: true
            };

            this.addSignal = function (nodeName, color) {
                color = color || nextColor();
                vm.nodeNames.push(nodeName);
                var series = new TimeSeries();
                if (!chart)
                    createTimeline();
                chart.addTimeSeries(series, {lineWidth: 2, strokeStyle: color});

                studioapi.getClient().find(nodeName).then(
                    function (node) {
                        var subscription = function (value, timestamp) {
                            var date = new Date();
                            series.append(date, value);
                        };
                        node.subscribeToValues(subscription);
                        vm.subscriptions.push({node: node, subscription: subscription});
                    }
                );
                return color;
            };

            var colors = ['blue', 'green', 'red', 'purple', 'black'];
            var colorIndex = 0;
            function nextColor() {
                return colors[colorIndex++ % colors.length];
            }

            function createTimeline() {
                var userOptions = $scope.$eval(vm.options);
                var mergedOptions = angular.merge(options, userOptions);
                chart = new SmoothieChart(mergedOptions);
                chart.streamTo($element.find('canvas')[0], 20);
            }

            function yRangeFunction(range) {
                if (this.min === null || isNaN(this.min))
                    this.min = range.min;
                if (this.max === null || isNaN(this.max))
                    this.max = range.max;

                this.min = Math.min(range.min, this.min);
                this.max = Math.max(range.max, this.max);

                var yRange = this.max - this.min;
                var extraSpace = yRange * 0.1; // Makes line more visible

                return {min: this.min - extraSpace, max: this.max + extraSpace};
            }

            this.$postLink = function () {
                setCanvasWidth();
            };

            this.$onDestroy = function () {
                for (var i = 0; i < vm.subscriptions.length; i++) {
                    var s = vm.subscriptions[i];
                    s.node.unsubscribeFromChildValues(s.subscription);
                }
            };

            function setCanvasWidth() {
                vm.canvasWidth = Math.min($element.parent()[0].offsetWidth, vm.width);
            }

            angular.element($window).bind('resize', function(){
                setCanvasWidth();
                $scope.$digest();
            });

        }],
        template: '\
          <div>\
            <canvas id="smoothieChart_{{::$id}}" width="{{vm.canvasWidth}}" height="{{vm.height}}"></canvas>\
            <div style="display: inline-block; vertical-align: top">\
              <span ng-transclude></span>\
            </div>\
          </div>'
    })
    .component('cdpChartLine', {
        require: {
            chartCtrl: '^cdpChart'
        },
        bindings: {
            routing: '@',
            color: '@'
        },
        controller: [function () {
            this.$onInit = function () {
                this.color = this.chartCtrl.addSignal(this.routing, this.color);
            };
        }],
        controllerAs: 'vm',
        template: '\
          <div ng-if="vm.chartCtrl.displayLegend !== false">\
            <div style="width: 8px;  height: 8px; margin-right: 5px; margin-left: 5px; background: {{vm.color}}; display: inline-block"></div> \
            <span>{{vm.routing}}</span>\
          </div>'
    });

var app = angular.module("angularCdp.cdp-input", []);

app.directive('cdpInput', ['studioapi', function(studioapi) {

    function link(scope, element, attrs) {

        var vm = scope;
        vm.subscription = {};

        studioapi.getClient().find(attrs.routing).then(
            function (node) {
                vm.subscription = {node: node, callback: function (value) {
                    scope.$evalAsync(function () {
                        scope.model = value;
                    });
                }};
                node.subscribeToValues(vm.subscription.callback);
            }
        );

        scope.onValueChange = function(newValue) {
            if (vm.subscription.node)
                vm.subscription.node.setValue(newValue);
        };

        element.on('$destroy', function() {
            if (vm.subscription.node) {
                var s = vm.subscription;
                s.node.unsubscribeFromChildValues(s.callback);
            }
        });
    }

    return {
        restrict: 'E',
        replace: true,
        scope: {
            routing: '@',
        },
        template: '<input ng-model="model" ng-change="onValueChange(model)">',
        link: link
    };
}]);

var app = angular.module("angularCdp.cdp-label", []);

app.directive('cdpLabel', ['studioapi', function(studioapi) {

    function link(scope, element, attrs) {

        var vm = scope;
        this.subscription = {};

        studioapi.getClient().find(attrs.routing).then(
            function (node) {
                var precision = attrs.precision;
                vm.subscription = {node: node, callback: function (value) {
                    if (precision && isNumeric(value))
                        value = value.toFixed(precision);
                    element.text(value);
                }};
                node.subscribeToValues(vm.subscription.callback);
            }
        );

        element.on('$destroy', function() {
            var s = vm.subscription;
            s.node.unsubscribeFromChildValues(s.callback);
        });

        function isNumeric(n) {
            return !isNaN(parseFloat(n)) && isFinite(n);
        }
    }

    return {
        restrict: 'E',
        scope: {
            routing: '=',
            precision: '='
        },
        template: '',
        link: link
    };
}]);
