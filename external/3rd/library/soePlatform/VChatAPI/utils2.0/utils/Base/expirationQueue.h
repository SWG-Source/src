#ifndef EXPIRATION_QUEUE_H
#define EXPIRATION_QUEUE_H

#include <time.h>
#include <map>
#include <set>

template<typename T, typename TimeType = time_t>
class expiration_queue
{
public:
	expiration_queue() { }
	virtual ~expiration_queue() { }

	bool empty() const { return m_expirationMap.empty(); }
	bool contains_expired_items(TimeType when) const;
	bool contains_item(const T & item) const;
	size_t size() const { return m_itemMap.size(); }
	const T & top() const;
	bool insert(const T & item, TimeType expiration_time);
	bool erase(const T & item);
	void clear();

private:
	typedef std::set<T> item_set_t;
	typedef std::map<TimeType , item_set_t> expiration_map_t;
	typedef std::map<T, TimeType> item_map_t;

	expiration_map_t m_expirationMap;
	item_map_t m_itemMap;
};

template<typename T, typename TimeType>
bool expiration_queue<T, TimeType>::contains_expired_items(TimeType when) const
{
	return (!m_expirationMap.empty() &&
			m_expirationMap.begin()->first <= when);
}

template<typename T, typename TimeType>
bool expiration_queue<T, TimeType>::contains_item(const T & item) const
{
	return (m_itemMap.find(item) != m_itemMap.end());
}

template<typename T, typename TimeType>
const T & expiration_queue<T, TimeType>::top() const
{
	static T dummy;

	if (!m_expirationMap.empty() && !m_expirationMap.begin()->second.empty()) {
		const item_set_t & firstSet = m_expirationMap.begin()->second;

		return *firstSet.begin();
	}

	return dummy;
}

template<typename T, typename TimeType>
bool expiration_queue<T, TimeType>::insert(const T & item, TimeType expiration_time)
{
	bool erased = erase(item);

	m_itemMap[item] = expiration_time;
	m_expirationMap[expiration_time].insert(item);

	return !erased;
}

template<typename T, typename TimeType>
bool expiration_queue<T, TimeType>::erase(const T & item)
{
	typename  item_map_t::iterator itIter = m_itemMap.find(item);
	bool erased = false;

	if (itIter != m_itemMap.end()) {
		typename expiration_map_t::iterator expIter = m_expirationMap.find(itIter->second);

		if (expIter != m_expirationMap.end()) {
			item_set_t & foundSet = expIter->second;

			erased = (foundSet.erase(item) > 0);

			if (foundSet.empty()) {
				m_expirationMap.erase(expIter);
			}
		}

		m_itemMap.erase(itIter);
	}
	
	return erased;
}

template<typename T, typename TimeType>
void expiration_queue<T, TimeType>::clear() 
{
	m_expirationMap.clear(); m_itemMap.clear();
}

#endif // EXPIRATION_QUEUE_H

