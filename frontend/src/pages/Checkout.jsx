import React, { useState, useEffect } from 'react';
import { Container, Row, Col, Card, Form, Button, Alert, Spinner } from 'react-bootstrap';
import { useNavigate } from 'react-router-dom';
import { useCart } from '../components/CartContext';
import { checkout } from '../services/api';
import toast from 'react-hot-toast';

const Checkout = () => {
  const { cartItems, getCartTotal, clearCart } = useCart();
  const navigate = useNavigate();

  const [formData, setFormData] = useState({
    name: '',
    phone: '',
    email: ''
  });
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(null);

  const subtotal = getCartTotal();
  const tax = subtotal * 0.08;
  const total = subtotal + tax;

  useEffect(() => {
    if (cartItems.length === 0) {
      navigate('/');
    }
  }, [cartItems, navigate]);

  const handleInputChange = (e) => {
    const { name, value } = e.target;
    setFormData(prev => ({ ...prev, [name]: value }));
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    setLoading(true);
    setError(null);

    try {
      const response = await checkout(cartItems, formData);
      if (response.success) {
        const orderId = response.orders[0].id;
        toast.success('Order placed successfully!');
        navigate(`/payment/${orderId}`);
      } else {
        setError(response.error || 'Checkout failed');
        toast.error(response.error || 'Checkout failed');
      }
    } catch (err) {
      setError('Network error. Please try again.');
      toast.error('Network error. Please try again.');
    } finally {
      setLoading(false);
    }
  };

  if (cartItems.length === 0 && !loading) {
    return (
      <Container className="mt-5 text-center">
        <h2>Your cart is empty</h2>
        <Button variant="primary" onClick={() => navigate('/')}>
          Browse Fish
        </Button>
      </Container>
    );
  }

  return (
    <Container className="py-4 fade-in">
      <Row className="justify-content-center">
        <Col lg={8}>
          <h2 className="mb-4">📋 Checkout</h2>

          {error && (
            <Alert variant="danger" onClose={() => setError(null)} dismissible>
              {error}
            </Alert>
          )}

          <Row className="g-4">
            <Col md={7}>
              <Card className="shadow-sm mb-4">
                <Card.Header className="bg-primary text-white">
                  <h5 className="mb-0">Customer Information</h5>
                </Card.Header>
                <Card.Body>
                  <Form onSubmit={handleSubmit}>
                    <Form.Group className="mb-3">
                      <Form.Label>Full Name *</Form.Label>
                      <Form.Control
                        type="text"
                        name="name"
                        value={formData.name}
                        onChange={handleInputChange}
                        placeholder="John Doe"
                        required
                      />
                    </Form.Group>

                    <Form.Group className="mb-3">
                      <Form.Label>Phone Number *</Form.Label>
                      <Form.Control
                        type="tel"
                        name="phone"
                        value={formData.phone}
                        onChange={handleInputChange}
                        placeholder="+1 (555) 123-4567"
                        required
                      />
                    </Form.Group>

                    <Form.Group className="mb-3">
                      <Form.Label>Email Address</Form.Label>
                      <Form.Control
                        type="email"
                        name="email"
                        value={formData.email}
                        onChange={handleInputChange}
                        placeholder="john@example.com"
                      />
                    </Form.Group>

                    <Button
                      variant="success"
                      type="submit"
                      size="lg"
                      className="w-100"
                      disabled={loading}
                    >
                      {loading ? (
                        <>
                          <Spinner animation="border" size="sm" className="me-2" />
                          Processing...
                        </>
                      ) : (
                        `Pay ₹₹${total.toFixed(2)}`
                      )}
                    </Button>
                  </Form>
                </Card.Body>
              </Card>
            </Col>

            <Col md={5}>
              <Card className="shadow-sm">
                <Card.Header>
                  <h5 className="mb-0">Order Summary</h5>
                </Card.Header>
                <Card.Body>
                  {cartItems.map(item => (
                    <div key={item.fishId} className="d-flex justify-content-between mb-2">
                      <span>{item.quantity}x {item.name}</span>
                      <span>₹${(item.price * item.quantity).toFixed(2)}</span>
                    </div>
                  ))}
                  <hr />
                  <div className="d-flex justify-content-between">
                    <span>Subtotal:</span>
                    <span>₹${subtotal.toFixed(2)}</span>
                  </div>
                  <div className="d-flex justify-content-between">
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
            </Col>
          </Row>
        </Col>
      </Row>
    </Container>
  );
};

export default Checkout;
