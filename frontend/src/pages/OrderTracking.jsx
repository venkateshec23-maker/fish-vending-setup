import React, { useState, useEffect } from 'react';
import { Container, Row, Col, Card, Badge, Spinner, Button, Alert } from 'react-bootstrap';
import { getCustomerHistory, deleteOrder } from '../services/api';
import toast from 'react-hot-toast';

const OrderTracking = () => {
  const [orders, setOrders] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    fetchOrders();
  }, []);

  const fetchOrders = async () => {
    try {
      const response = await getCustomerHistory();
      if (response.success) {
        setOrders(response.data);
      }
    } catch (err) {
      console.error('Error fetching orders:', err);
    } finally {
      setLoading(false);
    }
  };

  const handleDelete = async (orderId) => {
    if (!window.confirm('Are you sure you want to delete this order?')) return;
    try {
      const res = await deleteOrder(orderId);
      if (res.success) {
        toast.success('Order deleted');
        setOrders(prev => prev.filter(o => o.id !== orderId));
      } else {
        toast.error(res.error || 'Failed to delete order');
      }
    } catch (err) {
      toast.error('Error deleting order');
    }
  };

  const getStatusColor = (status) => {
    switch (status) {
      case 'pending': return 'warning';
      case 'processing': return 'info';
      case 'dispensing': return 'primary';
      case 'completed': return 'success';
      case 'cancelled': return 'danger';
      default: return 'secondary';
    }
  };

  const getPaymentStatusColor = (status) => {
    switch (status) {
      case 'completed': return 'success';
      case 'pending': return 'warning';
      case 'failed': return 'danger';
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

  return (
    <Container className="py-4 fade-in">
      <h2 className="mb-4">📋 My Order History</h2>

      {orders.length === 0 ? (
        <Alert variant="info" className="text-center">
          <Alert.Heading>No Orders Yet</Alert.Heading>
          <p>Start shopping to see your order history here.</p>
        </Alert>
      ) : (
        <Row className="g-4">
          {orders.map(order => (
            <Col key={order.id} xs={12} md={6} lg={4}>
              <Card className="shadow-sm h-100">
                <Card.Header className="d-flex justify-content-between align-items-center">
                  <Badge bg="primary">{order.id}</Badge>
                  <Badge bg={getStatusColor(order.status)}>
                    {order.status}
                  </Badge>
                </Card.Header>
                <Card.Body>
                  <Card.Title>{order.fish_name}</Card.Title>
                  <Card.Text>
                    {order.compartment && (
                      <div className="d-flex justify-content-between mb-2">
                        <span>Compartment:</span>
                        <span>#{order.compartment}</span>
                      </div>
                    )}
                    <div className="d-flex justify-content-between mb-2">
                      <span>Quantity:</span>
                      <span>{order.quantity}</span>
                    </div>
                    <div className="d-flex justify-content-between mb-2">
                      <span>Total:</span>
                      <span className="fw-bold">₹{order.total_price.toFixed(2)}</span>
                    </div>
                    <div className="d-flex justify-content-between mb-2">
                      <span>Payment:</span>
                      <Badge bg={getPaymentStatusColor(order.payment_status)}>
                        {order.payment_status}
                      </Badge>
                    </div>
                    <div className="d-flex justify-content-between text-muted small">
                      <span>Date:</span>
                      <span>{new Date(order.created_at).toLocaleDateString()}</span>
                    </div>
                  </Card.Text>
                </Card.Body>
                <Card.Footer className="bg-white">
                  <div className="d-grid gap-2">
                    <Button
                      variant="outline-primary"
                      size="sm"
                      href={`/order/${order.id}`}
                    >
                      View Details
                    </Button>
                    <Button
                      variant="outline-danger"
                      size="sm"
                      onClick={() => handleDelete(order.id)}
                    >
                      Delete Order
                    </Button>
                  </div>
                </Card.Footer>
              </Card>
            </Col>
          ))}
        </Row>
      )}
    </Container>
  );
};

export default OrderTracking;
