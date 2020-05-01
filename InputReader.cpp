#include "InputReader.h"

InputReader* StaticInputReaderReference = nullptr;

static void StaticOnArmPinInterrupt()
{
	StaticInputReaderReference->OnArmPinInterrupt();
}

void InputReader::AttachInterrupt()
{
	StaticInputReaderReference = this;
	attachInterrupt(ArmInterruptPin, StaticOnArmPinInterrupt, CHANGE);
}