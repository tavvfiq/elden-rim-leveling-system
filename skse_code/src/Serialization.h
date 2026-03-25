#pragma once

namespace Persist
{
	void Install();

	std::int32_t GetUnspentPoints();
	void SetUnspentPoints(std::int32_t value);

	std::int32_t GetERLevel();
	void SetERLevel(std::int32_t value);
}

