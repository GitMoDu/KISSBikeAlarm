// 
// 
// 

#include "InputReader.h"


InputReader* StaticReference = nullptr;

static void StaticOnDisarmPinInterrupt()
{
	StaticReference->OnDisarmPinInterrupt();
}

void InputReader::ArmInterrupt()
{
	StaticReference = this;
	attachInterrupt(digitalPinToInterrupt(ArmPin), StaticOnDisarmPinInterrupt, CHANGE);
}