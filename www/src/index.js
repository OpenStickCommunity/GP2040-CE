import React from 'react';
import * as ReactDOMClient from 'react-dom/client';
import App from './App';

import './index.scss';
import 'bootstrap/dist/js/bootstrap.bundle.min.js';

const container = document.getElementById('root');
const root = ReactDOMClient.createRoot(container);
root.render(<App />);
