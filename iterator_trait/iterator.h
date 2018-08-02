#pragma once

#include "option.h"

#include <iterator>

template<class It> struct StdFwdIteratorConsumer;
template<class It, class F> struct IterMap;
template<class It> struct IterEnumerate;

// The powerhouse of the cell
template<class I, class Derived>
struct Iterator {
	using Item = I;

	Option<Item> next() { return static_cast<Derived*>(this)->Derived::next(); }

	StdFwdIteratorConsumer<Derived> std_consumer()    { return StdFwdIteratorConsumer<Derived>{ static_cast<Derived&&>(*this) }; }
	StdFwdIteratorConsumer<Derived> begin()           { return std_consumer(); }
	StdFwdIteratorConsumer<Derived> end() const       { return StdFwdIteratorConsumer<Derived>{ None }; }

	template<class F> IterMap<Derived, F> map(F&& f)  { return IterMap<Derived, F>(static_cast<Derived&&>(*this), std::forward<F>(f)); }
	IterEnumerate<Derived> enumerate()                { return IterEnumerate<Derived>(static_cast<Derived&&>(*this)); }

	template<class A, class F>
	auto fold(A a, F&& f) -> A {
		// TODO: should A be && or is pass-by-value fine?
		// Also assumes `a` is assignable
		while(auto o = next()) {
			a = f(a, o.unwrap());
			// TODO: move?
		}

		return a;
	}
};


template<class It>
struct StdFwdIteratorConsumer {
	StdFwdIteratorConsumer(NoneType) : m_iter{ None }, m_item{ None } {}
	StdFwdIteratorConsumer(It&& i) : m_iter{ Some(std::move(i)) }, m_item{ m_iter.unwrap_ref().next() } {
		if (!m_item.is_some())
			m_iter = None;
	}

	StdFwdIteratorConsumer(StdFwdIteratorConsumer&& o) : m_iter{ std::move(o.m_iter) }, m_item{ std::move(o.m_item) } {}
	StdFwdIteratorConsumer& operator=(StdFwdIteratorConsumer&& o) {
		m_iter = std::move(o.m_iter);
		m_item = std::move(o.m_item);
	}

	// TODO: I don't know if returning rvalue-ref here is actually a reasonable thing to do
	StdFwdIteratorConsumer&& begin() { return std::move(*this); }
	StdFwdIteratorConsumer end() const { return StdFwdIteratorConsumer{ None }; }

	bool operator!= (const StdFwdIteratorConsumer& o) const {
		if (m_iter && o.m_iter) { std::abort(); }
		return m_iter.is_some() != o.m_iter.is_some();
	}

	StdFwdIteratorConsumer& operator++() {
		m_item = m_iter.unwrap_ref().next();
		if (!m_item.is_some())
			m_iter = None;

		return *this;
	}

	typename It::Item operator*() {
		if (!m_iter.is_some()) std::abort();
		return m_item.unwrap();
	}

private:
	Option<It> m_iter;
	Option<typename It::Item> m_item;
};


template<class It, class F>
struct IterMap : Iterator<typename It::Item, IterMap<It, F>> {
	using Item = decltype(std::declval<F>()(std::declval<typename It::Item>()));

	IterMap(It&& i, F&& f) : m_iter{std::move(i)}, m_func{std::forward<F>(f)} {}

	Option<Item> next() {
		if (auto o = m_iter.next()) {
			return Some(m_func(o.unwrap()));
		}

		return None;
	}

private:
	It m_iter;
	F m_func;
};


template<class It>
struct IterEnumerate : Iterator<std::pair<std::size_t, typename It::Item>, IterEnumerate<It>> {
	using Item = std::pair<std::size_t, typename It::Item>;

	IterEnumerate(It&& i) : m_iter{std::move(i)}, m_idx{0} {}

	Option<Item> next() {
		if (auto o = m_iter.next()) {
			return Some(Item(m_idx++, o.unwrap()));
		}

		return None;
	}

private:
	It m_iter;
	std::size_t m_idx;
};