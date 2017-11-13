/**
MoverDemoLibBuilder implementation.
*/

#include "Controller.h"
#include "MoverDemoLibBuilder.h"

using namespace MoverDemoLib;

MoverDemoLibBuilder::MoverDemoLibBuilder(const char* libName, const char* timeStamp)
    : CDPBuilder(libName, timeStamp)
{
}

CDPComponent* MoverDemoLibBuilder::CreateNewComponent(const std::string& type)
{
    if (type=="MoverDemoLib.Controller")
        return new Controller;
    
    return CDPBuilder::CreateNewComponent(type);
}

CDPBaseObject* MoverDemoLibBuilder::CreateNewCDPOperator(const std::string& modelName, const std::string& type, const CDPPropertyBase* inputProperty)
{
    return CDPBuilder::CreateNewCDPOperator(modelName, type, inputProperty);
}
