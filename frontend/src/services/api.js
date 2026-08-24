// Smart Fish Vending Machine - API Service
// Centralized API calls to the Flask backend

import axios from 'axios';

const API_BASE_URL = import.meta.env.VITE_API_URL || 'http://localhost:5000/api';

const api = axios.create({
  baseURL: API_BASE_URL,
  headers: {
    'Content-Type': 'application/json',
  },
});

// ==================== FISH / INVENTORY ====================

export const getFish = async () => {
  const response = await api.get('/fish');
  return response.data;
};

export const getFishById = async (id) => {
  const response = await api.get(`/fish/${id}`);
  return response.data;
};

// ==================== CART / CHECKOUT ====================

export const checkout = async (cartItems, customerInfo) => {
  const response = await api.post('/checkout', {
    items: cartItems,
    customerName: customerInfo.name,
    customerPhone: customerInfo.phone,
  });
  return response.data;
};

export const processPayment = async (orderId) => {
  const response = await api.post('/payment/process', { orderId });
  return response.data;
};

// ==================== ORDERS ====================

export const getOrder = async (orderId) => {
  const response = await api.get(`/order/${orderId}`);
  return response.data;
};

export const getAllOrders = async (status = null) => {
  const params = status ? { status } : {};
  const response = await api.get('/orders', { params });
  return response.data;
};

export const completeOrder = async (orderId) => {
  const response = await api.post(`/order/${orderId}/complete`);
  return response.data;
};

export const deleteOrder = async (orderId) => {
  const response = await api.delete(`/order/${orderId}`);
  return response.data;
};

// ==================== DASHBOARD ====================

export const getDashboardStats = async () => {
  const response = await api.get('/dashboard/stats');
  return response.data;
};

export const getRevenueData = async () => {
  const response = await api.get('/dashboard/revenue');
  return response.data;
};

export const getFishStats = async () => {
  const response = await api.get('/dashboard/fish-stats');
  return response.data;
};

export const getCustomerHistory = async () => {
  const response = await api.get('/customer/history');
  return response.data;
};

export default api;
