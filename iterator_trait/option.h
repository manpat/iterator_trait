#pragma once

#include <memory>
#include <cstdlib>
#include <type_traits>

template<class T, bool Copyable = std::is_copy_constructible<T>::value>
struct Option;

struct NoneType {
	explicit constexpr NoneType() {}

	template<class T>
	constexpr operator Option<T>() const;
	constexpr operator bool() const { return false; }
};

static constexpr NoneType None = NoneType{};
	struct PartialConstruct {};

template<class T, bool Copyable>
struct Option {
	using Item = std::remove_const_t<T>;

	constexpr explicit Option(T&& v) 	: m_value{ std::move(v) }, m_valid{true} {}
	constexpr          Option(NoneType) : m_null_value{}, m_valid{false} {}

	template<class TT>
	Option(const Option<TT, true>& o) : m_valid{o.m_valid} {
		if (m_valid)
			new (&m_value) T ( o.m_value );
	}

	template<class TT>
	Option(Option<TT>&& o) : m_valid{o.m_valid} {
		if (m_valid)
			new (&m_value) T ( std::move(o.m_value) );

		o.m_valid = false;
	}

	Option& operator=(const Option& o) {
		if (m_valid) { m_value.~T(); }
		
		m_valid = o.m_valid;
		if (m_valid)
			new (&m_value) T ( o.m_value );

		return *this;
	}

	Option& operator=(Option&& o) {
		if (m_valid) { m_value.~T(); }

		m_valid = o.m_valid;
		if (m_valid) {
			new (&m_value) T ( std::move(o.m_value) );
			o.m_valid = false;
		}

		return *this;
	}

	Option& operator=(NoneType) {
		if (m_valid) {
			m_value.~T();
		}

		m_valid = false;
		return *this;
	}

	~Option() { if (m_valid) { m_value.~T(); } m_valid = false; }

	bool      	is_some() const { return m_valid; }
	operator 	bool() const { return m_valid; }

	bool operator==(NoneType) const { return !m_valid; }

	T         unwrap() const { if (!is_some()) std::abort(); return m_value; }
	T&        unwrap_ref() { if (!is_some()) std::abort(); return m_value; }
	T         unwrap_or(T&& t) const { if (!is_some()) return t; return m_value; }

protected:
	Option(bool v, PartialConstruct) : m_valid{v} {}

	union {
		Item        m_value;
		NoneType    m_null_value;
	};

	bool   m_valid;
};


template<class T>
struct Option<T, false> : Option<T, true> {
	using Option<T, true>::Option;

	template<class TT, bool C>
	Option(const Option<TT, C>& o) = delete;

	template<bool C>
	Option& operator=(const Option<T, C>& o) = delete;
};


template<class T>
constexpr NoneType::operator Option<T>() const {
	return Option<T>{None};
}


template<class T>
constexpr Option<T> Some(T&& t) { return Option<T>{std::forward<T>(t)}; }

