import React, { useState, useEffect } from 'react';
import { Container, Row, Col, Card, Badge, Spinner, Button } from 'react-bootstrap';
import { useParams, useNavigate } from 'react-router-dom';
import { getOrder } from '../services/api';
import toast from 'react-hot-toast';

const OrderConfirmation = () => {
  const { orderId } = useParams();
  const navigate = useNavigate();
  const [order, setOrder] = useState(null);
  const [loading, setLoading] = useState(true);
  const [dispensing, setDispensing] = useState(false);

  useEffect(() => {
    fetchOrder();
  }, [orderId]);

  const fetchOrder = async () => {
    try {
      const response = await getOrder(orderId);
      if (response.success) {
        setOrder(response.data);
      }
    } catch (err) {
      toast.error('Error fetching order details');
    } finally {
      setLoading(false);
    }
  };

  const handleDispense = async () => {
    setDispensing(true);
    try {
      const response = await fetch('http://localhost:5000/dispense', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          orderId: order.id,
          fishType: order.fish_type,
          qty: order.quantity
        })
      });
      const data = await response.json();
      if (data.success) {
        toast.success('Dispensing started! Please collect your fish.');
      }
    } catch (err) {
      toast.error('Dispense request failed');
    } finally {
      setDispensing(false);
    }
  };

  const getStatusColor = (status) => {
    switch (status) {
      case 'pending': return 'warning';
      case 'processing': return 'info';
      case 'dispensing': return 'primary';
      case 'completed': return 'success';
      default: return 'secondary';
    }
  };

  if (loading) {
    return (
      <div className="spinner-overlay">
        <Spinner animation="border" variant="primary" />
      </div>
    );
  }

  if (!order) {
    return (
      <Container className="mt-5 text-center">
        <h2>Order not found</h2>
        <Button variant="primary" onClick={() => navigate('/')}>
          Go Home
        </Button>
      </Container>
    );
  }

  return (
    <Container className="py-5 fade-in">
      <Row className="justify-content-center">
        <Col lg={8}>
          <Card className="shadow">
            <Card.Header className="bg-success text-white text-center">
              <h3 className="mb-0">✅ Order Confirmed</h3>
            </Card.Header>
            <Card.Body className="p-4">
              <div className="text-center mb-4">
                <Badge bg={getStatusColor(order.status)} className="p-3 fs-6">
                  Status: {order.status.toUpperCase()}
                </Badge>
              </div>

              <Card className="mb-4 bg-light">
                <Card.Body>
                  <Row>
                    <Col md={6}>
                      <p><strong>Order ID:</strong></p>
                      <p className="fs-5 text-primary">{order.id}</p>
                    </Col>
                    <Col md={6}>
                      <p><strong>Date:</strong></p>
                      <p>{new Date(order.created_at).toLocaleString()}</p>
                    </Col>
                  </Row>
                  <hr />
                  <Row>
                    <Col md={6}>
                      <p><strong>Fish Type:</strong> {order.fish_name}</p>
                      {order.compartment && <p><strong>Compartment:</strong> #{order.compartment}</p>}
                      <p><strong>Quantity:</strong> {order.quantity}</p>
                    </Col>
                    <Col md={6}>
                      <p><strong>Total Price:</strong></p>
                      <p className="fs-4 fw-bold text-success">₹{order.total_price.toFixed(2)}</p>
                    </Col>
                  </Row>
                  {order.customer_name && (
                    <>
                      <hr />
                      <p><strong>Customer:</strong> {order.customer_name}</p>
                      {order.customer_phone && (
                        <p><strong>Phone:</strong> {order.customer_phone}</p>
                      )}
                    </>
                  )}
                </Card.Body>
              </Card>

              {order.status === 'processing' && (
                <div className="d-grid gap-2">
                  <Button
                    variant="primary"
                    size="lg"
                    onClick={handleDispense}
                    disabled={dispensing}
                  >
                    {dispensing ? (
                      <>
                        <Spinner animation="border" size="sm" className="me-2" />
                        Dispensing...
                      </>
                    ) : (
                      '🐟 Dispense Fish'
                    )}
                  </Button>
                </div>
              )}

              <div className="d-grid gap-2 mt-3">
                <Button variant="outline-primary" onClick={() => navigate('/orders')}>
                  View All Orders
                </Button>
                <Button variant="outline-secondary" onClick={() => navigate('/')}>
                  Continue Shopping
                </Button>
              </div>
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </Container>
  );
};

export default OrderConfirmation;
