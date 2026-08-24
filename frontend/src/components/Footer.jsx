import React from 'react';
import { Container, Row, Col, Card } from 'react-bootstrap';

const Footer = () => {
  return (
    <footer className="bg-dark text-white py-4 mt-auto">
      <Container>
        <Row>
          <Col md={6}>
            <h5>🐟 Smart Fish Vending Machine</h5>
            <p className="text-muted mb-0">
              Fresh fish, vending machine style. Sustainable, convenient, and always available.
            </p>
          </Col>
          <Col md={3} className="mt-3 mt-md-0">
            <h6>Contact</h6>
            <p className="text-muted mb-0 small">
              support@fishvending.com<br />
              +1 (555) 123-4567
            </p>
          </Col>
          <Col md={3} className="mt-3 mt-md-0">
            <h6>Hours</h6>
            <p className="text-muted mb-0 small">
              Open 24/7<br />
              Fresh restocked daily
            </p>
          </Col>
        </Row>
        <hr className="my-3" />
        <Row>
          <Col className="text-center text-muted small">
            © {new Date().getFullYear()} Smart Fish Vending Machine. All rights reserved.
          </Col>
        </Row>
      </Container>
    </footer>
  );
};

export default Footer;
