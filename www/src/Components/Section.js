import React from 'react';
import './Section.scss';

const Section = ({ children, title, ...props }) => {
	return (
		<div className={`card ${props.className}`}>
			<div className={`card-header ${props.titleClassName}`}>
				<strong>{title}</strong>
			</div>
			<div className="card-body">
				{children}
			</div>
		</div>
	);
};

export default Section;
