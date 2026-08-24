import React, { useState, useEffect } from 'react';
import { Container, Row, Col, Card, Button, Spinner, Badge, ProgressBar } from 'react-bootstrap';
import { useParams, useNavigate } from 'react-router-dom';
import { getOrder, processPayment } from '../services/api';
import toast from 'react-hot-toast';

const Payment = () => {
  const { orderId } = useParams();
  const navigate = useNavigate();
  const [order, setOrder] = useState(null);
  const [loading, setLoading] = useState(true);
  const [processing, setProcessing] = useState(false);
  const [paymentSuccess, setPaymentSuccess] = useState(false);

  useEffect(() => {
    fetchOrder();
  }, [orderId]);

  const fetchOrder = async () => {
    try {
      const response = await getOrder(orderId);
      if (response.success) {
        setOrder(response.data);
      } else {
        toast.error('Order not found');
        navigate('/');
      }
    } catch (err) {
      toast.error('Error fetching order');
      navigate('/');
    } finally {
      setLoading(false);
    }
  };

  const handlePayment = async () => {
    setProcessing(true);
    try {
      const response = await processPayment(orderId);
      if (response.success) {
        setPaymentSuccess(true);
        toast.success('Dispensing Started! Your order is being prepared.');
      } else {
        const errorMsg = response.error || 'Payment failed';
        if (response.retry_allowed) {
          toast.error(`${errorMsg} - Please try again or contact support.`);
        } else {
          toast.error(errorMsg);
        }
      }
    } catch (err) {
      toast.error('Payment processing error');
    } finally {
      setProcessing(false);
    }
  };

  const triggerDispense = async () => {
    // Dispensing is now handled by the backend after successful payment
    // This function is kept for backward compatibility
    console.log('Dispensing is handled by the backend');
  };

  if (loading) {
    return (
      <div className="spinner-overlay">
        <Spinner animation="border" variant="primary" />
      </div>
    );
  }

  if (paymentSuccess) {
    return (
      <Container className="py-5">
        <Card className="payment-success shadow">
          <Card.Body>
            <div className="success-icon">✓</div>
            <h2 className="text-success">Payment Successful!</h2>
            <p className="text-muted">Your order is being processed</p>
            <Card className="text-start bg-light">
              <Card.Body>
                <p><strong>Order ID:</strong> {orderId}</p>
                <p><strong>Fish:</strong> {order?.fish_name}</p>
                <p><strong>Quantity:</strong> {order?.quantity}</p>
                <p><strong>Total:</strong> ₹${order?.total_price?.toFixed(2)}</p>
              </Card.Body>
            </Card>
            <div className="d-grid gap-2 mt-4">
              <Button variant="primary" size="lg" onClick={() => navigate(`/order/${orderId}`)}>
                Track Order
              </Button>
              <Button variant="outline-primary" onClick={() => navigate('/')}>
                Continue Shopping
              </Button>
            </div>
          </Card.Body>
        </Card>
      </Container>
    );
  }

  return (
    <Container className="py-5">
      <Row className="justify-content-center">
        <Col lg={6}>
          <Card className="shadow">
            <Card.Header className="bg-warning">
              <h4 className="mb-0">💳 Fake Payment Gateway</h4>
            </Card.Header>
            <Card.Body className="p-4">
              <Alert variant="info">
                <strong>Demo Mode:</strong> This is a simulated payment page for demonstration.
              </Alert>

              <Card className="mb-4 bg-light">
                <Card.Body>
                  <h5>Order Summary</h5>
                  <div className="d-flex justify-content-between mb-2">
                    <span>Order ID:</span>
                    <Badge bg="primary">{orderId}</Badge>
                  </div>
                  <div className="d-flex justify-content-between mb-2">
                    <span>Fish:</span>
                    <span>{order?.fish_name}</span>
                  </div>
                  <div className="d-flex justify-content-between mb-2">
                    <span>Quantity:</span>
                    <span>{order?.quantity}</span>
                  </div>
                  <div className="d-flex justify-content-between fw-bold fs-5">
                    <span>Total:</span>
                    <span>₹${order?.total_price?.toFixed(2)}</span>
                  </div>
                </Card.Body>
              </Card>

              <Form>
                <Row className="g-3">
                  <Col md={6}>
                    <Form.Group>
                      <Form.Label>Card Number</Form.Label>
                      <Form.Control placeholder="4242 4242 4242 4242" readOnly />
                    </Form.Group>
                  </Col>
                  <Col md={3}>
                    <Form.Group>
                      <Form.Label>Expiry</Form.Label>
                      <Form.Control placeholder="12/25" readOnly />
                    </Form.Group>
                  </Col>
                  <Col md={3}>
                    <Form.Group>
                      <Form.Label>CVC</Form.Label>
                      <Form.Control placeholder="123" readOnly />
                    </Form.Group>
                  </Col>
                </Row>

                <Button
                  variant="success"
                  size="lg"
                  className="w-100 mt-4"
                  onClick={handlePayment}
                  disabled={processing}
                >
                  {processing ? (
                    <>
                      <Spinner animation="border" size="sm" className="me-2" />
                      Processing...
                    </>
                  ) : (
                    `Pay ₹${order?.total_price?.toFixed(2)}`
                  )}
                </Button>
              </Form>
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </Container>
  );
};

export default Payment;
