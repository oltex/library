#pragma once
#include "../pool/pool.h"

namespace library::data_structure {
	template<typename type, typename allocator = pool<type>, bool placement = true>
	class stack {
	private:
		using size_type = unsigned int;
		struct node final {
			inline explicit node(void) noexcept = delete;
			inline explicit node(node const&) noexcept = delete;
			inline explicit node(node&&) noexcept = delete;
			inline auto operator=(node const&) noexcept = delete;
			inline auto operator=(node&&) noexcept = delete;
			inline ~node(void) noexcept = delete;
			node* _next;
			type _value;
		};
		using rebind_allocator = allocator::template rebind<node>;
	public:
		inline explicit stack(void) noexcept
			: _head(nullptr) {
		};
		inline explicit stack(stack const& rhs) noexcept = delete;
		inline explicit stack(stack&& rhs) noexcept = delete;
		inline auto operator=(stack const& rhs) noexcept -> stack & = delete;
		inline auto operator=(stack&& rhs) noexcept -> stack & = delete;
		inline ~stack(void) noexcept {
			node* current = _head;
			while (nullptr != current) {
				node* next = current->_next;
				if constexpr (true == placement)
					system::memory::destruct(current->_value);
				current = next;
			}
		};

		template<typename... argument>
		inline void emplace(argument&&... arg) noexcept {
			node* current = &_allocator.allocate();
			if constexpr (true == placement)
				system::memory::construct<type>(current->_value, std::forward<argument>(arg)...);

			current->_next = _head;
			_head = current;
			++_size;
		}
		inline void pop(void) noexcept {
			node* current = _head;
			_head = current->_next;
			if constexpr (true == placement)
				system::memory::destruct(current->_value);
			_allocator.deallocate(*current);
			--_size;
		}

		inline auto top(void) noexcept -> type& {
			return _head->_value;
		}
		inline auto size(void) const noexcept -> size_type {
			return _size;
		}
		inline auto empty(void) const noexcept -> bool {
			return  0 == _size;
		}
	private:
		node* _head;
		rebind_allocator _allocator;
		size_type _size = 0;
	};
}