#pragma once




namespace salgo {



//namespace _ {
	enum class Iteration_Callback_Result {
		BREAK    = 0x4c8b3311, // magic numbers in case user forgets to return CONTINUE
		CONTINUE = 0x69f012b4
	};
//}
static constexpr auto BREAK    = Iteration_Callback_Result::BREAK;
static constexpr auto CONTINUE = Iteration_Callback_Result::CONTINUE;





} // namespace salgo
