/** xDDTools */
#pragma once

#include "xdd/proto.hpp"

namespace xdd {

namespace fun {

	template <typename T, typename F>
	void each(const std::vector<T>& vec, const F& functor)
	{
		std::for_each(vec.begin(), vec.end(), functor);
	}

	template <typename T, typename F>
	void transform(std::vector<T>& vec, const F& functor)
	{
		for (std::vector<T>::iterator i = vec.begin(); i != vec.end(); ++i)
		{
			functor(*i);
		}
	}


} // namespace fun

} // namespace xdd