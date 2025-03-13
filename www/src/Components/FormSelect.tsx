import { Form, FormSelectProps } from 'react-bootstrap';

type FormSelectTypes = {
	label?: string;
	error?: string;
	groupClassName?: string;
	hidden?: boolean;
} & FormSelectProps;

const FormSelect = ({
	label,
	error,
	groupClassName = '',
	hidden = false,
	...props
}: FormSelectTypes) => {
	return (
		<Form.Group className={groupClassName} hidden={hidden}>
			{label && <Form.Label>{label}</Form.Label>}
			<Form.Select {...props} />
			<Form.Control.Feedback type="invalid">{error}</Form.Control.Feedback>
		</Form.Group>
	);
};

export default FormSelect;
