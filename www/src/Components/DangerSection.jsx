import Section from './Section';

const DangerSection = ({ className, titleClassName, ...props }) => {
	return (
		<Section
			className={`border-danger ${className}`}
			titleClassName={`text-white bg-danger ${titleClassName}`}
			{...props}
		/>
	);
};

export default DangerSection;
