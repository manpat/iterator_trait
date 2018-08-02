#pragma once

#include "iterator.h"

#include <type_traits>

template<class StdBeginIt, class StdEndIt>
struct InputIteratorAdaptor : Iterator<
	typename std::iterator_traits<StdBeginIt>::value_type,
	InputIteratorAdaptor<StdBeginIt, StdEndIt>> {

	// std::iterator_traits<StdBeginIt>::value_type removes cv qualifiers
	using Item = std::remove_reference_t<typename std::iterator_traits<StdBeginIt>::reference>;

	InputIteratorAdaptor(StdBeginIt&& b, StdEndIt&& e)
		: m_begin{ std::forward<StdBeginIt>(b) }
		, m_end{ std::forward<StdEndIt>(e) }
		, m_valid{ true } {}

	InputIteratorAdaptor(InputIteratorAdaptor&& o) : m_valid{ o.m_valid } {
		if (m_valid) {
			new (&m_begin) StdBeginIt { std::forward<StdBeginIt>(o.m_begin) };
			new (&m_end) StdEndIt { std::forward<StdEndIt>(o.m_end) };
			o.m_valid = false;
		}
	}

	~InputIteratorAdaptor() {
		if (m_valid) {
			m_begin.~StdBeginIt();
			m_end.~StdEndIt();
			m_valid = false;
		}
	}

	Option<Item> next() {
		if (!m_valid) return None;

		if (m_begin != m_end) {
			Option<Item> r(std::move(*m_begin));
			++m_begin;
			return r;
		}

		return None;
	}

	union { StdBeginIt m_begin; };
	union { StdEndIt m_end; };
	bool m_valid;
};

template<class StdBeginIt, class StdEndIt>
auto iter(StdBeginIt&& b, StdEndIt&& e) -> InputIteratorAdaptor<StdBeginIt, StdEndIt> {
	return{ std::forward<StdBeginIt>(b), std::forward<StdEndIt>(e) };
}

template<class StdContainer>
auto iter(StdContainer&& c) -> InputIteratorAdaptor<decltype(std::begin(c)), decltype(std::end(c))> {
	return{ std::begin(c), std::end(c) };
}