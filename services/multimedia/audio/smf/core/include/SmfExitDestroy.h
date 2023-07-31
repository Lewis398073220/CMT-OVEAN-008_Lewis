#pragma once
#include "smf_api.h"
#include "smf_api_pool.h"

namespace smf {
	class ExitDestroy {
	private:
		void* _hd=0;
	public:
		ExitDestroy(void* hd) :_hd(hd) {}
		~ExitDestroy() {
			if (_hd)smf_destroy(_hd);
		}
		void Reset(void* hd = 0) {
			_hd = hd;
		}
	};

	class ExitFree{
	private:
		void* _hd=0;
		void* _ptr=0;
	public:
		ExitFree(void* hd,void*ptr) :_hd(hd),_ptr(ptr) {}
		~ExitFree() {
			if (_hd && _ptr)smf_free(_hd,_ptr);
		}
		void Reset(void* ptr = 0) {
			_ptr = ptr;
		}
	};

	template<class T>
	class ExitDelete {
	private:
		T* _hd = 0;
	public:
		ExitDelete(T* hd) :_hd(hd) {}
		~ExitDelete() {
			if (_hd)delete _hd;
		}
		void Reset(T* hd = 0) {
			_hd = hd;
		}
	};

	template<class T>
	class ExitDeletes {
	private:
		T* _hd = 0;
	public:
		ExitDeletes(T* hd) :_hd(hd) {}
		~ExitDeletes() {
			if (_hd)delete[] _hd;
		}
		void Reset(T* hd = 0) {
			_hd = hd;
		}
	};
}