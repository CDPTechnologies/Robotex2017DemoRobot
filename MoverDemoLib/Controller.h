/**
 * (c)2017 CDP Technologies AS
 */

#ifndef MOVERDEMOLIB_CONTROLLER_H
#define MOVERDEMOLIB_CONTROLLER_H

#include <CDPSystem/Base/CDPComponent.h>

namespace MoverDemoLib {

class Controller : public CDPComponent
{
public:
  Controller();
  ~Controller() override;

  void Create(const char* fullName) override;
  void CreateModel() override;
  void Configure(const char* componentXML) override;
  void Activate() override;

  void ProcessFollowLeftWall();
  void ProcessFollowRightWall();
  void ProcessDrive();
  void ProcessTurn();

  bool TransitionFromNullToDrive();
  bool TransitionFromTurnToDrive();
  bool TransitionToDrive();
  bool TransitionToTurn();
  bool TransitionToFollowRightWall();
  bool TransitionToFollowLeftWall();

  bool CanSeeFrontWall();
  bool CanSeeLeftWall();
  bool CanSeeRightWall();

protected:
  CDPSignal<double> MotorLeft;
  CDPSignal<double> MotorRight;
  CDPSignal<double> SensorLeft;
  CDPSignal<double> SensorRight;
  CDPSignal<double> PIDLeft;
  CDPSignal<double> PIDRight;
  CDPSignal<double> SensorFront;
  CDPProperty<double> BaseSpeed;
  CDPProperty<double> DistanceFromWall;
  CDPProperty<double> SensorLeftWallThreshold;
  CDPProperty<double> SensorRightWallThreshold;
  CDPProperty<double> SensorFrontWallThreshold;
  CDPProperty<double> DriveStraightSpeed;
  CDPProperty<double> TurnSpeed;
  CDPTimer m_transtitionToDriveTimer;
  using CDPComponent::ts;
  using CDPComponent::fs;
};

}

#endif
