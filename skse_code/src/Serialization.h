#pragma once

#include "Attributes.h"

namespace Persist
{
	void Install();

	std::int32_t GetUnspentPoints();
	void SetUnspentPoints(std::int32_t value);

	std::int32_t GetERLevel();
	void SetERLevel(std::int32_t value);

	ER::AttributeSet GetAttributes();
	void SetAttributes(const ER::AttributeSet& value);
}

