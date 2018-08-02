#pragma once

#include "option.h"

#include <iterator>

template<class It> struct StdFwdIteratorConsumer;
template<class It, class F> struct IterMap;
template<class It, class F> struct IterFilter;
template<class It, class It2> struct IterZip;
template<class It, class It2> struct IterChain;
template<class It> struct IterEnumerate;

// The powerhouse of the cell
template<class I, class Derived>
struct Iterator {
	// TODO: Do we actually need to pass item type as a type parameter?

	// You MUST define an Item type
	// using Item = I;

	// You MUST implement this
	Option<void> next() = delete;

	// TODO: Optionally implement this?
	std::pair<size_t, Option<size_t>> size_hint() = delete;

	Derived&& move_this() { return static_cast<Derived&&>(*this); }
	Derived* super_this() { return static_cast<Derived*>(this); }

	// NOTE: All methods from here on take ownership of this (excluding end())
	// this Iterator should not be used afterwards, it is potentially UB to do so
	StdFwdIteratorConsumer<Derived> std_consumer()         { return StdFwdIteratorConsumer<Derived>{ move_this() }; }
	StdFwdIteratorConsumer<Derived> begin()                { return std_consumer(); }
	StdFwdIteratorConsumer<Derived> end() const            { return StdFwdIteratorConsumer<Derived>{ None }; }

	template<class F> IterMap<Derived, F> map(F&& f)       { return IterMap<Derived, F>{ move_this(), std::forward<F>(f) }; }
	template<class F> IterFilter<Derived, F> filter(F&& f) { return IterFilter<Derived, F>{ move_this(), std::forward<F>(f) }; }
	IterEnumerate<Derived> enumerate()                     { return IterEnumerate<Derived>{ move_this() }; }

	// 'It' is always moved
	template<class It, class ItNoRef = std::remove_reference_t<It>>
	IterZip<Derived, ItNoRef> zip(It&& it)                 { return IterZip<Derived, ItNoRef>{ move_this(), std::move(it) }; }
	template<class It, class ItNoRef = std::remove_reference_t<It>>
	IterChain<Derived, ItNoRef> chain(It&& it)             { return IterChain<Derived, ItNoRef>{ move_this(), std::move(it) }; }

	template<class A, class F>
	auto fold(A a, F&& f) -> A {
		// TODO: should A be && or is pass-by-value fine?
		// Also assumes `a` is assignable
		while(auto o = super_this()->next()) {
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
struct IterMap : Iterator<decltype(std::declval<F>()(std::declval<typename It::Item>())), IterMap<It, F>> {
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


template<class It, class F>
struct IterFilter : Iterator<typename It::Item, IterFilter<It, F>> {
	using Item = typename It::Item;

	IterFilter(It&& i, F&& f) : m_iter{std::move(i)}, m_func{std::forward<F>(f)} {}

	Option<Item> next() {
		while (auto o = m_iter.next()) {
			if(m_func(o.unwrap_ref()))
				return Some(o.unwrap());
		}

		return None;
	}

private:
	It m_iter;
	F m_func;
};


template<class It, class It2>
struct IterZip : Iterator<std::pair<typename It::Item, typename It2::Item>, IterZip<It, It2>> {
	using Item = std::pair<typename It::Item, typename It2::Item>;

	IterZip(It&& i, It2&& i2) : m_iter{std::move(i)}, m_iter2{std::move(i2)} {}

	Option<Item> next() {
		auto a = m_iter.next();
		auto b = m_iter2.next();
		if (a && b) {
			return Some(Item(a.unwrap(), b.unwrap()));
		}

		return None;
	}

private:
	It m_iter;
	It2 m_iter2;
};


template<class It, class It2>
struct IterChain : Iterator<std::common_type_t<typename It::Item, typename It2::Item>, IterChain<It, It2>> {
	using Item = std::common_type_t<typename It::Item, typename It2::Item>;

	IterChain(It&& i, It2&& i2) : m_iter{std::move(i)}, m_iter2{std::move(i2)} {}

	Option<Item> next() {
		if(auto a = m_iter.next()) {
			return Some(Item(a.unwrap()));
		}

		if(auto b = m_iter2.next()) {
			return Some(Item(b.unwrap()));
		}

		return None;
	}

private:
	It m_iter;
	It2 m_iter2;
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
