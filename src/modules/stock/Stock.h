/**
 * @file
 */

#pragma once

#include "Item.h"
#include "Inventory.h"
#include <vector>
#include <unordered_map>
#include <memory>

namespace stock {

class ContainerProvider;
typedef std::shared_ptr<ContainerProvider> ContainerProviderPtr;

class StockDataProvider;
typedef std::shared_ptr<StockDataProvider> StockProviderPtr;

/**
 * @defgroup Stock
 * @{
 * @brief The Stock class manages Items. All the items that someone owns are stored in this class.
 *
 * The stock handler is taking responsibility for putting the items into it's Inventory. The Inventory itself
 * only has access to pointers over all of the items.
 */
class Stock {
private:
	/** All the items this instance can deal with */
	std::unordered_map<ItemId, ItemPtr> _items;
	/** The inventory has pointers to all the items distributed over all the Container instances in the Inventory. */
	Inventory _inventory;
	StockProviderPtr _stockDataProvider;

	inline auto find(const ItemId& itemId) const {
		return _items.find(itemId);
	}

	inline auto find(const ItemId& itemId) {
		return _items.find(itemId);
	}
public:
	Stock(const StockProviderPtr& stockDataProvider);

	/**
	 * @brief Initializes the stock and the inventory.
	 */
	bool init();

	/**
	 * @brief Adds a new item to the stock
	 * @param[in] item The @c Item to add.
	 */
	ItemPtr add(const ItemPtr& item);

	/**
	 * @brief Removes a particular amount of items
	 * @return The remaining amount
	 */
	int remove(const ItemPtr& item);

	/**
	 * @brief Count how many items of the given @c ItemType are in the @c Stock
	 */
	int count(const ItemType& itemType) const;

	/**
	 * @brief Count how many items of the given @c ItemId are in the @c Stock
	 */
	int count(ItemId itemId) const;

	const Inventory& inventory() const;
	Inventory& inventory();
};

inline const Inventory& Stock::inventory() const {
	return _inventory;
}

inline Inventory& Stock::inventory() {
	return _inventory;
}

/**
 * @}
 */

}
