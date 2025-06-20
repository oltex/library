#pragma once
#include <Windows.h>

namespace window {
	class object {
	public:
		inline explicit object(HGDIOBJ hgdiobj) noexcept
			: _hgdiobj(hgdiobj) {
		};
		inline explicit object(object const& rhs) noexcept = delete;
		inline explicit object(object&& rhs) noexcept
			: _hgdiobj(rhs._hgdiobj) {
			rhs._hgdiobj = nullptr;
		};
		inline auto operator=(object const& rhs) noexcept -> object & = delete;
		inline auto operator=(object&& rhs) noexcept -> object & = delete;
		inline virtual ~object(void) noexcept {
			DeleteObject(_hgdiobj);
		}
	public:
		inline auto data(void) noexcept -> HGDIOBJ& {
			return _hgdiobj;
		}
	protected:
		HGDIOBJ _hgdiobj;
	};
}