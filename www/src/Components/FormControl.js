import React from 'react';
import { Form } from 'react-bootstrap';

const FormControl = ({ label, error, groupClassName, ...props }) => {
	return (
		<Form.Group className={groupClassName}>
			<Form.Label>{label}</Form.Label>
			<Form.Control {...props} />
			<Form.Control.Feedback type="invalid">{error}</Form.Control.Feedback>
		</Form.Group>
	);
};

export default FormControl;
