/**
 * (c)2017 CDP Technologies AS
 */

#include "Controller.h"

using namespace MoverDemoLib;

const double TRANSITION_TO_DRIVE_TIMEOUT_SEC = 3.0;
const double SMOOTH_DRIVE_SPEED = 10.0;

Controller::Controller()
{
}

Controller::~Controller()
{
}

void Controller::Create(const char* fullName)
{
  CDPComponent::Create(fullName);
  SmoothDrive.Create("SmoothDrive",this,CDPPropertyBase::e_Element,(CDPOBJECT_SETPROPERTY_HANDLER)nullptr,(CDPOBJECT_VALIDATEPROPERTY_HANDLER)nullptr);
  MotorsOn.Create("MotorsOn",this,CDPPropertyBase::e_Element,(CDPOBJECT_SETPROPERTY_HANDLER)nullptr,(CDPOBJECT_VALIDATEPROPERTY_HANDLER)nullptr);
  PowerButtonPressed.Create("PowerButtonPressed",this);
  LightButtonPressed.Create("LightButtonPressed",this);
  DistanceFromWall.Create("DistanceFromWall",this,CDPPropertyBase::e_Element);
  DriveStraightSpeed.Create("DriveStraightSpeed",this,CDPPropertyBase::e_Element);
  TurnSpeed.Create("TurnSpeed",this,CDPPropertyBase::e_Element);
  BaseSpeed.Create("BaseSpeed",this,CDPPropertyBase::e_Element);
  MotorLeft.Create("MotorLeft",this);
  MotorRight.Create("MotorRight",this);
  SensorLeft.Create("SensorLeft",this);
  SensorRight.Create("SensorRight",this);
  PIDLeft.Create("PIDLeft",this);
  PIDRight.Create("PIDRight",this);
  SensorFront.Create("SensorFront",this);
  SensorFrontWallThreshold.Create("SensorFrontWallThreshold",this,CDPPropertyBase::e_Element);
  SensorLeftWallThreshold.Create("SensorLeftWallThreshold",this,CDPPropertyBase::e_Element);
  SensorRightWallThreshold.Create("SensorRightWallThreshold",this,CDPPropertyBase::e_Element);
}

void Controller::CreateModel()
{
  CDPComponent::CreateModel();

  RegisterStateProcess("Null", (CDPCOMPONENT_STATEPROCESS)&Controller::ProcessNull, "Initial Null state");
  RegisterStateProcess("FollowLeftWall",(CDPCOMPONENT_STATEPROCESS)&Controller::ProcessFollowLeftWall,"Use left sensor to follow wall");
  RegisterStateProcess("FollowRightWall",(CDPCOMPONENT_STATEPROCESS)&Controller::ProcessFollowRightWall,"Use right sensor to follow wall");
  RegisterStateProcess("Drive",(CDPCOMPONENT_STATEPROCESS)&Controller::ProcessDrive,"Drive straight");
  RegisterStateProcess("Turn",(CDPCOMPONENT_STATEPROCESS)&Controller::ProcessTurn,"Turn until front sensor can't see any walls");
  RegisterStateTransitionHandler("Drive","FollowLeftWall",(CDPCOMPONENT_STATETRANSITIONHANDLER)&Controller::TransitionToFollowLeftWall,"");
  RegisterStateTransitionHandler("Drive","Turn",(CDPCOMPONENT_STATETRANSITIONHANDLER)&Controller::TransitionToTurn,"");
  RegisterStateTransitionHandler("Drive","FollowRightWall",(CDPCOMPONENT_STATETRANSITIONHANDLER)&Controller::TransitionToFollowRightWall,"");
  RegisterStateTransitionHandler("Null","Drive",(CDPCOMPONENT_STATETRANSITIONHANDLER)&Controller::TransitionFromNullToDrive,"");
  RegisterStateTransitionHandler("Turn","Drive",(CDPCOMPONENT_STATETRANSITIONHANDLER)&Controller::TransitionFromTurnToDrive,"");
  RegisterStateTransitionHandler("Turn","FollowLeftWall",(CDPCOMPONENT_STATETRANSITIONHANDLER)&Controller::TransitionToFollowLeftWall,"");
  RegisterStateTransitionHandler("Turn","FollowRightWall",(CDPCOMPONENT_STATETRANSITIONHANDLER)&Controller::TransitionToFollowRightWall,"");
  RegisterStateTransitionHandler("FollowRightWall","Drive",(CDPCOMPONENT_STATETRANSITIONHANDLER)&Controller::TransitionToDrive,"");
  RegisterStateTransitionHandler("FollowRightWall","Turn",(CDPCOMPONENT_STATETRANSITIONHANDLER)&Controller::TransitionToTurn,"");
  RegisterStateTransitionHandler("FollowLeftWall","Drive",(CDPCOMPONENT_STATETRANSITIONHANDLER)&Controller::TransitionToDrive,"");
  RegisterStateTransitionHandler("FollowLeftWall","Turn",(CDPCOMPONENT_STATETRANSITIONHANDLER)&Controller::TransitionToTurn,"");
  // Transitions between FollowRightWall and FollowLeftWall removed for stability.
}

void Controller::Configure(const char* componentXML)
{
  CDPComponent::Configure(componentXML);

  SmoothDrive.SetPropertyChangeHandler([&] (CDPPropertyBase*) {
      if (SmoothDrive)
          BaseSpeed = SMOOTH_DRIVE_SPEED;
  });
  PowerButtonPressed.GetPropertyObject("Value")->SetPropertyChangeHandler([&] (CDPPropertyBase*) {
      if (PowerButtonPressed)
          MotorsOn = !MotorsOn;
  });
  LightButtonPressed.GetPropertyObject("Value")->SetPropertyChangeHandler([&] (CDPPropertyBase*) {
      if (LightButtonPressed)
          SmoothDrive = !SmoothDrive;
  });
}

void Controller::Activate()
{
  CDPComponent::Activate();
  m_transtitionToDriveTimer.Reset(TRANSITION_TO_DRIVE_TIMEOUT_SEC);
  m_transtitionToDriveTimer.Start();
}

void Controller::ProcessFollowLeftWall()
{
  MotorLeft = BaseSpeed - PIDLeft;
  MotorRight = BaseSpeed + PIDLeft;
}

void Controller::ProcessFollowRightWall()
{
  MotorLeft = BaseSpeed + PIDRight;
  MotorRight = BaseSpeed - PIDRight;
}

void Controller::ProcessDrive()
{
  MotorLeft = MotorRight = static_cast<double>(DriveStraightSpeed);
}

void Controller::ProcessTurn()
{
  if (SensorLeft > SensorRight)
  {
    MotorLeft = static_cast<double>(TurnSpeed);
    MotorRight = 0;
  }
  else
  {
    MotorLeft = 0;
    MotorRight = static_cast<double>(TurnSpeed);
  }
}

bool Controller::TransitionFromNullToDrive()
{
  return true;
}

/**
 * Change state only if robot has not seen any wall TRANSITION_TO_DRIVE_TIMEOUT_SEC seconds.
 */
bool Controller::TransitionToDrive()
{
  if (!CanSeeFrontWall() && !CanSeeLeftWall() && !CanSeeRightWall())
  {
    if (m_transtitionToDriveTimer.TimedOut())
      return true;
  }
  else
    m_transtitionToDriveTimer.Restart();
  return false;
}

bool Controller::TransitionFromTurnToDrive()
{
  return !CanSeeFrontWall() && !CanSeeLeftWall() && !CanSeeRightWall();
}

bool Controller::TransitionToTurn()
{
  return CanSeeFrontWall();
}

bool Controller::TransitionToFollowRightWall()
{
  return !CanSeeFrontWall() && CanSeeRightWall() && SensorRight >= SensorLeft;
}

bool Controller::TransitionToFollowLeftWall()
{
  return !CanSeeFrontWall() && CanSeeLeftWall() && SensorLeft > SensorRight;
}

bool Controller::CanSeeFrontWall()
{
  if (SmoothDrive)
    return SensorFront > SensorFrontWallThreshold && SensorFront > std::max(SensorLeft, SensorRight);
  else
    return SensorFront > SensorFrontWallThreshold;
}

bool Controller::CanSeeLeftWall()
{
  return SensorLeft > SensorLeftWallThreshold;
}

bool Controller::CanSeeRightWall()
{
  return SensorRight > SensorRightWallThreshold;
}
