import React, { useState, useEffect, createContext, useContext } from 'react';

// Cart Context for global state management
const CartContext = createContext();

export const useCart = () => {
  const context = useContext(CartContext);
  if (!context) {
    throw new Error('useCart must be used within a CartProvider');
  }
  return context;
};

export const CartProvider = ({ children }) => {
  const [cartItems, setCartItems] = useState(() => {
    const saved = localStorage.getItem('fishVendingCart');
    return saved ? JSON.parse(saved) : [];
  });
  const [isCartOpen, setIsCartOpen] = useState(false);

  useEffect(() => {
    localStorage.setItem('fishVendingCart', JSON.stringify(cartItems));
  }, [cartItems]);

  const addToCart = (fish) => {
    setCartItems(prev => {
      const existing = prev.find(item => item.fishId === fish.id);
      if (existing) {
        return prev;
      }
      return [...prev, {
        fishId: fish.id,
        name: fish.name,
        price: fish.price,
        quantity: 1,
        image_url: fish.image_url,
        maxQuantity: 1
      }];
    });
  };

  const removeFromCart = (fishId) => {
    setCartItems(prev => prev.filter(item => item.fishId !== fishId));
  };

  const updateQuantity = (fishId, quantity) => {
    if (quantity <= 0) {
      removeFromCart(fishId);
      return;
    }
    setCartItems(prev =>
      prev.map(item =>
        item.fishId === fishId
          ? { ...item, quantity: Math.min(quantity, item.maxQuantity) }
          : item
      )
    );
  };

  const clearCart = () => {
    setCartItems([]);
  };

  const getCartTotal = () => {
    return cartItems.reduce((total, item) => total + (item.price * item.quantity), 0);
  };

  const getCartCount = () => {
    return cartItems.reduce((count, item) => count + item.quantity, 0);
  };

  const toggleCart = () => {
    setIsCartOpen(prev => !prev);
  };

  return (
    <CartContext.Provider value={{
      cartItems,
      addToCart,
      removeFromCart,
      updateQuantity,
      clearCart,
      getCartTotal,
      getCartCount,
      isCartOpen,
      toggleCart,
      setIsCartOpen
    }}>
      {children}
    </CartContext.Provider>
  );
};
