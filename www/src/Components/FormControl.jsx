import React from 'react';
import { Form } from 'react-bootstrap';

const FormControl = ({
	onClick,
	label,
	error,
	groupClassName,
	hidden,
	labelClassName,
	...props
}) => {
	return (
		<Form.Group className={groupClassName} onClick={onClick} hidden={hidden}>
			<Form.Label className={labelClassName}>{label}</Form.Label>
			<Form.Control {...props} />
			<Form.Control.Feedback type="invalid">{error}</Form.Control.Feedback>
		</Form.Group>
	);
};

export default FormControl;
