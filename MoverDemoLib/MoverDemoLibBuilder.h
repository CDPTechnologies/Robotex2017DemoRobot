/**
MoverDemoLibBuilder header file.
*/

#ifndef MOVERDEMOLIB_MOVERDEMOLIBBUILDER_H
#define MOVERDEMOLIB_MOVERDEMOLIBBUILDER_H

#include <CDPSystem/Application/CDPBuilder.h>

namespace MoverDemoLib {

class MoverDemoLibBuilder : public CDPBuilder
{
public:
    MoverDemoLibBuilder(const char* libName,const char* timeStamp);
    CDPComponent* CreateNewComponent(const std::string& type) override;
    CDPBaseObject* CreateNewCDPOperator(const std::string& modelName,const std::string& type,const CDPPropertyBase* inputProperty) override;
};

}

#endif // MOVERDEMOLIB_MOVERDEMOLIBBUILDER_H
