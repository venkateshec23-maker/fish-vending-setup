import React, { useState, useEffect } from 'react';
import { Container, Row, Col, Card, Spinner, Alert, Button } from 'react-bootstrap';
import FishCard from '../components/FishCard';
import { getFish } from '../services/api';
import Cart from '../components/Cart';

const CustomerDashboard = () => {
  const [fishList, setFishList] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  useEffect(() => {
    fetchFish();
  }, []);

  const fetchFish = async () => {
    try {
      setLoading(true);
      const response = await getFish();
      if (response.success) {
        setFishList(response.data);
      } else {
        setError('Failed to load fish inventory');
      }
    } catch (err) {
      setError('Error fetching fish data. Please try again later.');
      console.error(err);
    } finally {
      setLoading(false);
    }
  };

  if (loading) {
    return (
      <div className="spinner-overlay">
        <Spinner animation="border" variant="primary" />
      </div>
    );
  }

  if (error) {
    return (
      <Container className="mt-5">
        <Alert variant="danger">
          <Alert.Heading>Oops!</Alert.Heading>
          <p>{error}</p>
          <Button variant="outline-danger" onClick={fetchFish}>
            Try Again
          </Button>
        </Alert>
      </Container>
    );
  }

  return (
    <Container className="py-4 fade-in">
      <div className="text-center mb-5">
        <h1 className="display-5 fw-bold text-primary">
          🐟 Fresh Fish Vending Machine
        </h1>
        <p className="lead text-muted">
          Select your fresh fish and complete your purchase in seconds
        </p>
      </div>

      <Row className="g-4">
        {fishList.map(fish => (
          <Col key={fish.id} xs={12} sm={6} md={6} lg={3}>
            <FishCard fish={fish} />
          </Col>
        ))}
      </Row>

      {fishList.length === 0 && (
        <Alert variant="info" className="text-center mt-5">
          <Alert.Heading>No Fish Available</Alert.Heading>
          <p>Please check back later for fresh inventory.</p>
        </Alert>
      )}

      <Cart />
    </Container>
  );
};

export default CustomerDashboard;
