#pragma once
#include <functional>

namespace dg {
	template <class T>
	class Cache {
		private:
			using Refl = std::function<void (T&)>;
			mutable T		_value;
			mutable bool	_dirty;
			Refl			_refl;

		public:
			template <class R>
			Cache(R&& refl):
				_dirty(true),
				_refl(std::forward<Refl>(refl))
			{}
			const T& get() const {
				if(_dirty) {
					_dirty = false;
					_refl(_value);
				}
				return _value;
			}
			void setDirty() {
				_dirty = true;
			}
	};
}
