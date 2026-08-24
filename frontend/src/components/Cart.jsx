import React from 'react';
import { Container, Row, Col, Card, Button, ListGroup, Form } from 'react-bootstrap';
import { useCart } from '../components/CartContext';
import { useNavigate } from 'react-router-dom';
import toast from 'react-hot-toast';

const Cart = () => {
  const { cartItems, removeFromCart, updateQuantity, getCartTotal, isCartOpen, setIsCartOpen, clearCart } = useCart();
  const navigate = useNavigate();

  if (!isCartOpen) return null;

  const handleCheckout = () => {
    if (cartItems.length === 0) {
      toast.error('Your cart is empty!');
      return;
    }
    setIsCartOpen(false);
    navigate('/checkout');
  };

  const subtotal = getCartTotal();
  const tax = subtotal * 0.08;
  const total = subtotal + tax;

  return (
    <>
      <div className="cart-overlay" onClick={() => setIsCartOpen(false)} />
      <div className="cart-sidebar p-4">
        <div className="d-flex justify-content-between align-items-center mb-4">
          <h4 className="mb-0">🛒 Your Cart</h4>
          <Button
            variant="outline-secondary"
            size="sm"
            onClick={() => setIsCartOpen(false)}
          >
            ✕
          </Button>
        </div>

        {cartItems.length === 0 ? (
          <div className="text-center py-5 text-muted">
            <div style={{ fontSize: '4rem' }}>🛒</div>
            <p className="mt-3">Your cart is empty</p>
            <Button
              variant="primary"
              onClick={() => setIsCartOpen(false)}
            >
              Browse Fish
            </Button>
          </div>
        ) : (
          <>
            <ListGroup className="mb-3" style={{ maxHeight: '400px', overflowY: 'auto' }}>
              {cartItems.map(item => (
                <ListGroup.Item key={item.fishId} className="d-flex justify-content-between align-items-center">
                  <div className="d-flex align-items-center gap-3">
                    <img
                      src={item.image_url || 'https://via.placeholder.com/50'}
                      alt={item.name}
                      style={{ width: '50px', height: '50px', objectFit: 'cover', borderRadius: '8px' }}
                    />
                    <div>
                      <h6 className="mb-0">{item.name}</h6>
                      <small className="text-muted">₹${item.price.toFixed(2)} each</small>
                    </div>
                  </div>
                  <div className="d-flex align-items-center gap-2">
                    <Form.Control
                      type="number"
                      min="1"
                      max="1"
                      value={item.quantity}
                      disabled
                      style={{ width: '60px', textAlign: 'center' }}
                      size="sm"
                    />
                    <Button
                      variant="outline-danger"
                      size="sm"
                      onClick={() => removeFromCart(item.fishId)}
                    >
                      ✕
                    </Button>
                  </div>
                </ListGroup.Item>
              ))}
            </ListGroup>

            <Card className="mb-3">
              <Card.Body>
                <div className="d-flex justify-content-between mb-2">
                  <span>Subtotal:</span>
                  <span>₹${subtotal.toFixed(2)}</span>
                </div>
                <div className="d-flex justify-content-between mb-2">
                  <span>Tax (8%):</span>
                  <span>₹${tax.toFixed(2)}</span>
                </div>
                <hr />
                <div className="d-flex justify-content-between fw-bold fs-5">
                  <span>Total:</span>
                  <span>₹${total.toFixed(2)}</span>
                </div>
              </Card.Body>
            </Card>

            <div className="d-grid gap-2">
              <Button variant="success" size="lg" onClick={handleCheckout}>
                Proceed to Checkout
              </Button>
              <Button variant="outline-secondary" onClick={clearCart}>
                Clear Cart
              </Button>
            </div>
          </>
        )}
      </div>
    </>
  );
};

export default Cart;
