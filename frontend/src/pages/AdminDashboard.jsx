import React, { useState, useEffect } from 'react';
import { Container, Row, Col, Card, Table, Spinner, Badge, ProgressBar } from 'react-bootstrap';
import { getDashboardStats, getRevenueData, getFishStats } from '../services/api';

const AdminDashboard = () => {
  const [stats, setStats] = useState(null);
  const [revenueData, setRevenueData] = useState([]);
  const [fishStats, setFishStats] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    fetchDashboardData();
  }, []);

  const fetchDashboardData = async () => {
    try {
      const [statsRes, revenueRes, fishStatsRes] = await Promise.all([
        getDashboardStats(),
        getRevenueData(),
        getFishStats()
      ]);

      if (statsRes.success) setStats(statsRes.data);
      if (revenueRes.success) setRevenueData(revenueRes.data);
      if (fishStatsRes.success) setFishStats(fishStatsRes.data);
    } catch (err) {
      console.error('Error fetching dashboard data:', err);
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

  if (!stats) {
    return (
      <Container className="mt-5">
        <Alert variant="danger">Failed to load dashboard data</Alert>
      </Container>
    );
  }

  return (
    <Container fluid className="py-4 fade-in">
      <h2 className="mb-4">📊 Admin Dashboard</h2>

      {/* Stats Cards */}
      <Row className="g-4 mb-4">
        <Col xs={12} sm={6} lg={3}>
          <Card className="admin-card shadow-sm border-0 bg-primary text-white">
            <Card.Body>
              <Card.Title as="h6" className="text-uppercase text-white-50">
                Total Revenue
              </Card.Title>
              <div className="stat-value">₹${stats.total_revenue.toFixed(2)}</div>
              <Card.Text className="mb-0 text-white-50">
                All time earnings
              </Card.Text>
            </Card.Body>
          </Card>
        </Col>

        <Col xs={12} sm={6} lg={3}>
          <Card className="admin-card shadow-sm border-0 bg-success text-white">
            <Card.Body>
              <Card.Title as="h6" className="text-uppercase text-white-50">
                Total Orders
              </Card.Title>
              <div className="stat-value">{stats.total_orders}</div>
              <Card.Text className="mb-0 text-white-50">
                Completed transactions
              </Card.Text>
            </Card.Body>
          </Card>
        </Col>

        <Col xs={12} sm={6} lg={3}>
          <Card className="admin-card shadow-sm border-0 bg-info text-white">
            <Card.Body>
              <Card.Title as="h6" className="text-uppercase text-white-50">
                Fish Sold
              </Card.Title>
              <div className="stat-value">{stats.total_fish_sold}</div>
              <Card.Text className="mb-0 text-white-50">
                Units sold
              </Card.Text>
            </Card.Body>
          </Card>
        </Col>

        <Col xs={12} sm={6} lg={3}>
          <Card className="admin-card shadow-sm border-0 bg-warning text-dark">
            <Card.Body>
              <Card.Title as="h6" className="text-uppercase">
                Inventory
              </Card.Title>
              <div className="stat-value">{stats.available_inventory}</div>
              <Card.Text className="mb-0">
                Units remaining
              </Card.Text>
            </Card.Body>
          </Card>
        </Col>
      </Row>

      {/* Machine Status */}
      <Row className="g-4 mb-4">
        <Col xs={12}>
          <Card className="shadow-sm">
            <Card.Header className="bg-dark text-white">
              <h5 className="mb-0">🖥️ Machine Status</h5>
            </Card.Header>
            <Card.Body>
              <Row>
                <Col md={3}>
                  <div className="text-center p-3">
                    <div className={`display-6 ${stats.machine_online ? 'text-success' : 'text-danger'}`}>
                      {stats.machine_online ? '🟢' : '🔴'}
                    </div>
                    <h5>{stats.machine_online ? 'Online' : 'Offline'}</h5>
                  </div>
                </Col>
                {stats.machine_status && (
                  <>
                    <Col md={3}>
                      <div className="text-center p-3">
                        <div className="display-6 text-info">🌡️</div>
                        <h5>{stats.machine_status.temperature}°C</h5>
                        <small className="text-muted">Temperature</small>
                      </div>
                    </Col>
                    <Col md={3}>
                      <div className="text-center p-3">
                        <div className="display-6 text-primary">💧</div>
                        <h5>{stats.machine_status.humidity}%</h5>
                        <small className="text-muted">Humidity</small>
                      </div>
                    </Col>
                    <Col md={3}>
                      <div className="text-center p-3">
                        <div className="display-6 text-success">🐟</div>
                        <h5>{stats.fish_types}</h5>
                        <small className="text-muted">Fish Types</small>
                      </div>
                    </Col>
                  </>
                )}
              </Row>
            </Card.Body>
          </Card>
        </Col>
      </Row>

      {/* Revenue Chart (Last 7 Days) */}
      <Row className="g-4 mb-4">
        <Col xs={12} lg={8}>
          <Card className="shadow-sm">
            <Card.Header>
              <h5 className="mb-0">📈 Revenue (Last 7 Days)</h5>
            </Card.Header>
            <Card.Body>
              {revenueData.length === 0 ? (
                <p className="text-muted text-center">No revenue data available</p>
              ) : (
                <div className="d-flex flex-column gap-3">
                  {revenueData.map((day, index) => (
                    <div key={index}>
                      <div className="d-flex justify-content-between mb-1">
                        <small>{new Date(day.date).toLocaleDateString()}</small>
                        <small className="fw-bold">₹${day.revenue.toFixed(2)} ({day.orders} orders)</small>
                      </div>
                      <ProgressBar
                        now={(day.revenue / Math.max(...revenueData.map(d => d.revenue), 1)) * 100}
                        variant="primary"
                        label={day.revenue > 0 ? '' : 'No sales'}
                      />
                    </div>
                  ))}
                </div>
              )}
            </Card.Body>
          </Card>
        </Col>

        <Col xs={12} lg={4}>
          <Card className="shadow-sm">
            <Card.Header>
              <h5 className="mb-0">🐟 Fish Statistics</h5>
            </Card.Header>
            <Card.Body>
              {fishStats.length === 0 ? (
                <p className="text-muted">No fish data</p>
              ) : (
                <Table hover size="sm">
                  <thead>
                    <tr>
                      <th>Fish</th>
                      <th>Sold</th>
                      <th>Available</th>
                    </tr>
                  </thead>
                  <tbody>
                    {fishStats.map((fish, index) => (
                      <tr key={index}>
                        <td>{fish.name}</td>
                        <td>{fish.sold}</td>
                        <td>
                          <Badge bg={fish.available > 10 ? 'success' : fish.available > 0 ? 'warning' : 'danger'}>
                            {fish.available}
                          </Badge>
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </Table>
              )}
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </Container>
  );
};

export default AdminDashboard;
