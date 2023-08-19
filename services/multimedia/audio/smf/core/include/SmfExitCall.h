#pragma once
//#include <functional>
namespace smf {
	template<class TFunc>
	class ExitCall {
	protected:
		TFunc _func;
		bool _enable = true;
	public:
		ExitCall(const TFunc& func) :_func(func) {}
		~ExitCall() {
			if (_enable)_func();
		}
		void Reset() {
			_enable = false;
		}
	};

	template<class TFunc>
	class ExitCallx1 {
	protected:
		TFunc _func;
		void* _priv;
		bool _enable = true;
	public:
		ExitCallx1(const TFunc& func, void* priv) :_func(func), _priv(priv) {}
		~ExitCallx1() {
			if (_enable)_func(_priv);
		}
		void Reset() {
			_enable = false;
		}
	};
#if 0
	class ExitCall {
	private:
		std::function<void()> _func;
	public:
		ExitCall(const std::function<void()>& func) :_func(func) {}

		//template<class Func>
		//ExitCall(Func func) : _func([]() {func(); }) {}

		//template<class Func, class...PS>
		//ExitCall(Func func,PS...ps) : _func([]() {func(ps...); }) {}

		template<class Func, class T, class...PS>
		ExitCall(T& t, Func func, PS...ps) : _func([=]() {(t.*func)(ps...); }) {}

		template<class...PS>
		ExitCall(void (*func)(PS...ps),PS...ps) : _func([=]() {func(ps...); }) {}

		template<class T,class...PS>
		ExitCall(T& t, void (T::*func)(PS...ps),PS...ps) : _func([=]() {(t.*func)(ps...); }) {}

	public:
		~ExitCall() {
			if (_func)_func();
		}
	public:
		void Reset() {
			_func = NULL;
		}
		void CallAndReset() {
			if (_func)_func();
			_func = NULL;
		}
	};

	template<class TFunc>
	class SmfExitCall {
	protected:
		//using Func = typename TFunc;
		using Func = TFunc;
		Func _func;
	public:
		SmfExitCall(const Func& func) :_func(func) {}
		~SmfExitCall() {
			if (_func)_func();
		}
		void Reset() {
			_func = nullptr;
		}
	};

	typedef SmfExitCall<std::function<void()>> SmfExitCallx;
#endif
}