type JoyCon2ControllerProps = {
	side: 'left' | 'right';
	body: string;
	buttons: string;
	accent: string;
	stick: string;
	active?: 'body' | 'buttons' | 'accent' | 'stick' | null;
};

const JoyCon2Controller = ({
	side,
	body,
	buttons,
	accent,
	stick,
	active = null,
}: JoyCon2ControllerProps) => {
	const part = (name: JoyCon2ControllerProps['active']) => ({
		opacity: active && active !== name ? 0.2 : 1,
		style: { transition: 'opacity 150ms' },
	});
	return (
		<svg
			xmlns="http://www.w3.org/2000/svg"
			viewBox="0 0 320 140"
			fillRule="evenodd"
			width="100%"
			role="img"
		>
			{side === 'left' ? (
				<>
					<g fill={body} {...part('body')}>
						<path d="M100.3,125C35.4,123.9-0.2,95.4,4.3,30L318.3,30C321.1,85.4,300.5,123.2,230.3,125L100.3,125Z" />
					</g>
					<g fill={buttons} {...part('buttons')}>
						<path d="M76,123.1L75.4,126C24.3,120.5-0.7,88.3,1.6,41.8L3.9,41.8C4.3,90.8,30.2,116.2,76,123.1Z" />
						<rect x="212" y="47.1" width="16" height="16" />
						<rect x="36.7" y="39.1" width="5" height="16" />
						<circle cx="163.6" cy="98.8" r="11.5" />
						<circle cx="186.6" cy="75.8" r="11.5" />
						<circle cx="163.6" cy="52.8" r="11.5" />
						<circle cx="140.6" cy="75.8" r="11.5" />
					</g>
					<g fill={accent} {...part('accent')}>
						<rect x="15.3" y="14" width="290" height="16" />
						<path d="M83.3,48.5C98.2,48.5,110.3,60.6,110.3,75.5C110.3,90.4,98.2,102.5,83.3,102.5C68.4,102.5,56.3,90.4,56.3,75.5C56.3,60.6,68.4,48.5,83.3,48.5ZM83.3,53.9C71.4,53.9,61.7,63.5,61.7,75.5C61.7,87.4,71.4,97.1,83.3,97.1C95.2,97.1,104.9,87.4,104.9,75.5C104.9,63.5,95.2,53.9,83.3,53.9Z" />
					</g>
					<g fill={stick} {...part('stick')}>
						<circle cx="83.3" cy="75.5" r="22" />
					</g>
				</>
			) : (
				<>
					<g fill={body} {...part('body')}>
						<path d="M219.7,125C284.6,123.9,320.2,95.4,315.7,30L1.7,30C-1.1,85.4,19.5,123.2,89.7,125L219.7,125Z" />
					</g>
					<g fill={buttons} {...part('buttons')}>
						<path d="M244,123.1L244.6,126C295.7,120.5,320.7,88.3,318.4,41.8L316.1,41.8C315.7,90.8,289.8,116.2,244,123.1Z" />
						<rect x="59.7" y="50.3" width="16" height="16" />
						<path d="M278.8,46.1L278.8,40.7L283.8,40.7L283.8,46.1L289.3,46.1L289.3,51.1L283.8,51.1L283.8,56.6L278.8,56.6L278.8,51.1L273.3,51.1L273.3,46.1L278.8,46.1Z" />
						<circle cx="236.3" cy="100.3" r="11.5" />
						<circle cx="213.3" cy="77.3" r="11.5" />
						<circle cx="100" cy="58.3" r="11" />
						<circle cx="236.3" cy="54.3" r="11.5" />
						<circle cx="259.3" cy="77.3" r="11.5" />
					</g>
					<g fill={accent} {...part('accent')}>
						<rect x="14.7" y="14" width="290" height="16" />
						<path d="M156.9,50.3C142,50.3,129.9,62.4,129.9,77.3C129.9,92.2,142,104.3,156.9,104.3C171.8,104.3,183.9,92.2,183.9,77.3C183.9,62.4,171.8,50.3,156.9,50.3ZM156.9,55.7C168.8,55.7,178.5,65.4,178.5,77.3C178.5,89.2,168.8,98.9,156.9,98.9C145,98.9,135.3,89.2,135.3,77.3C135.3,65.4,145,55.7,156.9,55.7Z" />
					</g>
					<g fill={stick} {...part('stick')}>
						<circle cx="156.9" cy="77.3" r="22" />
					</g>
				</>
			)}
		</svg>
	);
};
export default JoyCon2Controller;
