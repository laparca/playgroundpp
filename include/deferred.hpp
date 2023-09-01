#pragma once

#include <utility>

namespace laparca {

/**
 * deferred runs the specified function when the variable get out of scope:
 * 
 * \code{.cpp}
 *    {
 *       deferred var{[]() { std::cout << "Godbye!" << std::endl; } };
 *       // do something
 *    } // variable var is out of scope. Its destructor is called and it run the specified code
 *      // In this case, "Godbye!" is printed
 * \endcode
 * 
 * Be careful to give a name to the defer variable. If it hasn't got one,
 * the variable is destroyed inmediately.
 * 
 * This class is non copyable nor movable nor copy constructible nor move constructibe.
 * 
 * \tparam F type of the function or callable class to use. It should be detected by the compiler.
*/
template<typename F>
struct deferred {
	constexpr deferred(const F& f) : f_(f) {}
	constexpr deferred(F&& f) : f_{std::move(f)} {}

	deferred(const deferred&) = delete;
	deferred(deferred&&) = delete;
	deferred& operator=(const deferred&) = delete;
	deferred& operator=(deferred&&) = delete;
	
	constexpr ~deferred() {
		f_();
	}
private:
	F f_;
};

}