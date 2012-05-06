/** xDDTools */
#pragma once

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
		std::transform(vec.begin(), vec.end(), functor);
	}

	template <typename T, typename F>
	T find_or_nullptr(const std::vector<T>& vec, const F& functor)
	{
		std::vector<T>::const_iterator i = vec.begin();
		for (; i != vec.end(); ++i)
		{
			if (functor(*i))
				return *i;
		}
		return nullptr;
	}


} // namespace fun

} // namespace xdd