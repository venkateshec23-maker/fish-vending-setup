import React from 'react';
import { Routes, Route, Navigate } from 'react-router-dom';
import { CartProvider } from './components/CartContext';
import NavigationBar from './components/Navbar';
import Footer from './components/Footer';
import Cart from './components/Cart';
import CustomerDashboard from './pages/CustomerDashboard';
import Checkout from './pages/Checkout';
import Payment from './pages/Payment';
import OrderConfirmation from './pages/OrderConfirmation';
import OrderTracking from './pages/OrderTracking';
import AdminDashboard from './pages/AdminDashboard';

const App = () => {
  return (
    <CartProvider>
      <div className="d-flex flex-column min-vh-100">
        <NavigationBar />
        <main className="flex-grow-1">
          <Routes>
            <Route path="/" element={<CustomerDashboard />} />
            <Route path="/checkout" element={<Checkout />} />
            <Route path="/payment/:orderId" element={<Payment />} />
            <Route path="/order/:orderId" element={<OrderConfirmation />} />
            <Route path="/orders" element={<OrderTracking />} />
            <Route path="/admin" element={<AdminDashboard />} />
            <Route path="*" element={<Navigate to="/" replace />} />
          </Routes>
        </main>
        <Footer />
        <Cart />
      </div>
    </CartProvider>
  );
};

export default App;
