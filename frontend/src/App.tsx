import {Routes, Route, Navigate} from 'react-router-dom';
import Login from './pages/Login';
import Dashboard from './pages/Dashboard';

function ProtectedRoute({children}:{children: React.ReactNode}) {
  const token = localStorage.getItem('token'); // TODO: use httpOnly cookie instead
  return token ? <>{children}</> : <Navigate to="/login" replace />
}

function App() {
  return (
    <Routes>
      <Route path="/login" element={<Login />} />
      <Route
        path="/dashboard"
        element={
          <ProtectedRoute>
            <Dashboard />
          </ProtectedRoute>
        }
      />
      <Route path="/" element={<Navigate to="/login" replace />} />
    </Routes>
  );
}

export default App;
