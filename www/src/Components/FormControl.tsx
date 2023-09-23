import React, { HTMLProps } from 'react';
import { Form, FormControlProps } from 'react-bootstrap';

type formTypes = {
	onClick?: () => void;
	label: string;
	error?: string;
	groupClassName?: string;
	hidden?: boolean;
} & FormControlProps &
	HTMLProps<HTMLInputElement>;

const FormControl = ({
	onClick,
	label,
	error,
	groupClassName,
	hidden = false,
	...props
}: formTypes) => {
	return (
		<Form.Group className={groupClassName} onClick={onClick} hidden={hidden}>
			<Form.Label>{label}</Form.Label>
			<Form.Control {...props} />
			<Form.Control.Feedback type="invalid">{error}</Form.Control.Feedback>
		</Form.Group>
	);
};

export default FormControl;
