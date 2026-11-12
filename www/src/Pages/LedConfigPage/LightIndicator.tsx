import { useDraggable } from '@dnd-kit/core';
import { OverlayTrigger, Popover } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import { Light } from '../../Store/useLedStore';
import { LIGHT_TYPES } from '../../Data/Leds';

type LightIndicatorProps = {
	id: number;
	cellWidth: number;
	active: boolean;
	error?: string;
} & Light;

export const LightIcon = ({
	size,
	active,
	lightType,
	error,
}: {
	size: number;
	lightType: number;
	active: boolean;
	error?: string;
}) => {
	switch (lightType) {
		case LIGHT_TYPES.ActionButton: {
			return (
				<svg
					xmlns="http://www.w3.org/2000/svg"
					width={size}
					height={size}
					viewBox="0 0 16 16"
				>
					<circle
						cx={8}
						cy={8}
						r={6}
						fill={error ? 'red' : active ? 'currentColor' : 'none'}
						stroke="currentColor"
						strokeWidth={1}
					/>
				</svg>
			);
		}
		case LIGHT_TYPES.Case: {
			return (
				<svg
					xmlns="http://www.w3.org/2000/svg"
					width={size}
					height={size}
					viewBox="0 0 16 16"
				>
					<rect
						x={2}
						y={2}
						width={12}
						height={12}
						rx={2}
						ry={2}
						fill={error ? 'red' : active ? 'currentColor' : 'none'}
						stroke="currentColor"
						strokeWidth={1}
					/>
				</svg>
			);
		}
		case LIGHT_TYPES.Turbo:
		case LIGHT_TYPES.PlayerLight: {
			return (
				<svg
					xmlns="http://www.w3.org/2000/svg"
					width={size}
					height={size}
					viewBox="0 0 16 16"
				>
					<polygon
						points="8,1 14,4.5 14,11.5 8,15 2,11.5 2,4.5"
						fill={error ? 'red' : active ? 'currentColor' : 'none'}
						stroke="currentColor"
						strokeWidth={1}
					/>
				</svg>
			);
		}
		default:
			return null;
	}
};

export function LightIndicator({
	id,
	active = false,
	cellWidth,
	yCoord,
	xCoord,
	numLedsOnLight,
	firstLedIndex,
	lightType,
	GPIOPinOrNonButtonIndex,
	error,
}: LightIndicatorProps) {
	const { t } = useTranslation('');
	const { attributes, isDragging, listeners, setNodeRef, transform } =
		useDraggable({ id });

	return (
		<div
			{...listeners}
			{...attributes}
			aria-label={t('LedConfigPage:lightIndicator.aria-label')}
			ref={setNodeRef}
			style={{
				width: cellWidth,
				height: cellWidth,
				transform: `translate(${transform?.x ?? 0}px, ${transform?.y ?? 0}px) scale(${
					isDragging ? 1.3 : 1.2
				})`,
				justifyContent: 'center',
				alignItems: 'center',
				display: 'flex',
				left: xCoord * cellWidth,
				top: yCoord * cellWidth,
				position: 'absolute',
				appearance: 'none',
				background: 'none',
				border: 'none',
				cursor: 'grab',
				clear: 'both',
			}}
		>
			{isDragging ? (
				<LightIcon
					lightType={lightType}
					size={cellWidth}
					active={active}
					error={error}
				/>
			) : (
				<OverlayTrigger
					key={`light-${id}`}
					placement="auto"
					rootClose
					overlay={
						<Popover>
							<Popover.Header as="h3">
								{t('LedConfigPage:lightIndicator.popover-header', {
									index: id + 1,
								})}
							</Popover.Header>
							<Popover.Body>
								<div style={{ minWidth: 200 }}>
									<div className="d-flex w-100 justify-content-between">
										<span className="text-secondary">
											{t('LedConfigPage:lightIndicator.gpio-case-label')}
										</span>
										<span>
											{lightType === LIGHT_TYPES.Case
												? GPIOPinOrNonButtonIndex + 1
												: GPIOPinOrNonButtonIndex}
										</span>
									</div>
									<div className="d-flex w-100 justify-content-between">
										<span className="text-secondary">
											{t('LedConfigPage:lightIndicator.type-label')}
										</span>
										<span>
											{lightType == LIGHT_TYPES.ActionButton &&
												t('LedConfigPage:lightIndicator.type-action-button')}
											{lightType === LIGHT_TYPES.Case &&
												t('LedConfigPage:lightIndicator.type-case')}
											{lightType === LIGHT_TYPES.Turbo &&
												t('LedConfigPage:lightIndicator.type-turbo')}
											{lightType === LIGHT_TYPES.PlayerLight &&
												t('LedConfigPage:lightIndicator.type-player-light')}
										</span>
									</div>
									<div className="d-flex w-100 justify-content-between">
										<span className="text-secondary">
											{t('LedConfigPage:lightIndicator.leds-label')}
										</span>
										<span>{numLedsOnLight}</span>
									</div>
									<div className="d-flex w-100 justify-content-between">
										<span className="text-secondary">
											{t('LedConfigPage:lightIndicator.coords-label')}
										</span>
										<span>
											({xCoord}, {yCoord})
										</span>
									</div>
									<div className="d-flex w-100 justify-content-between">
										<span className="text-secondary">
											{t('LedConfigPage:lightIndicator.first-led-label')}
										</span>
										<span>{firstLedIndex}</span>
									</div>
								</div>
							</Popover.Body>
						</Popover>
					}
				>
					<div>
						<LightIcon
							lightType={lightType}
							size={cellWidth}
							active={active}
							error={error}
						/>
					</div>
				</OverlayTrigger>
			)}
		</div>
	);
}
