import { useDraggable } from '@dnd-kit/core';
import { OverlayTrigger, Popover } from 'react-bootstrap';
import { Light } from '../../Store/useLedStore';

type LightIndicatorProps = {
	id: number;
	cellWidth: number;
	active: boolean;
	error?: string;
} & Light;

export const LightIcon = ({
	size,
	active,
	error,
}: {
	size: number;
	active: boolean;
	error?: string;
}) => (
	<div>
		{active ? (
			<svg
				xmlns="http://www.w3.org/2000/svg"
				width={size}
				height={size}
				fill={error ? 'red' : 'currentColor'}
				viewBox="0 0 16 16"
				style={{ filter: 'drop-shadow(0 4px 16px rgba(255, 255, 200, 0.7))' }}
			>
				<path d="M2 6a6 6 0 1 1 10.174 4.31c-.203.196-.359.4-.453.619l-.762 1.769A.5.5 0 0 1 10.5 13h-5a.5.5 0 0 1-.46-.302l-.761-1.77a2 2 0 0 0-.453-.618A5.98 5.98 0 0 1 2 6m3 8.5a.5.5 0 0 1 .5-.5h5a.5.5 0 0 1 0 1l-.224.447a1 1 0 0 1-.894.553H6.618a1 1 0 0 1-.894-.553L5.5 15a.5.5 0 0 1-.5-.5" />
			</svg>
		) : (
			<svg
				xmlns="http://www.w3.org/2000/svg"
				width={size}
				height={size}
				fill={error ? 'red' : 'currentColor'}
				viewBox="0 0 16 16"
				style={{ filter: 'drop-shadow(0 4px 16px rgba(255, 255, 200, 0.7))' }}
			>
				<path d="M2 6a6 6 0 1 1 10.174 4.31c-.203.196-.359.4-.453.619l-.762 1.769A.5.5 0 0 1 10.5 13a.5.5 0 0 1 0 1 .5.5 0 0 1 0 1l-.224.447a1 1 0 0 1-.894.553H6.618a1 1 0 0 1-.894-.553L5.5 15a.5.5 0 0 1 0-1 .5.5 0 0 1 0-1 .5.5 0 0 1-.46-.302l-.761-1.77a2 2 0 0 0-.453-.618A5.98 5.98 0 0 1 2 6m6-5a5 5 0 0 0-3.479 8.592c.263.254.514.564.676.941L5.83 12h4.342l.632-1.467c.162-.377.413-.687.676-.941A5 5 0 0 0 8 1" />
			</svg>
		)}
	</div>
);

export function LightIndicator({
	id,
	active = false,
	cellWidth,
	yCoord,
	xCoord,
	numLedsOnLight,
	firstLedIndex,
	lightType,
	GPIOPinorCaseChainIndex,
	error,
}: LightIndicatorProps) {
	const { attributes, isDragging, listeners, setNodeRef, transform } =
		useDraggable({ id });

	return (
		<div
			{...listeners}
			aria-label="Draggable Light Indicator"
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
				<LightIcon size={cellWidth} active={active} error={error} />
			) : (
				<OverlayTrigger
					key={`light-${id}`}
					placement="auto"
					rootClose
					overlay={
						<Popover>
							<Popover.Header as="h3">Light {id + 1}</Popover.Header>
							<Popover.Body>
								<div style={{ minWidth: 200 }}>
									<div className="d-flex w-100 justify-content-between">
										<span className="text-secondary">GPIO/Case:</span>
										<span>
											{lightType === 1
												? GPIOPinorCaseChainIndex + 1
												: GPIOPinorCaseChainIndex}
										</span>
									</div>
									<div className="d-flex w-100 justify-content-between">
										<span className="text-secondary">Type:</span>
										<span>
											{lightType === 0 && 'ActionButton'}
											{lightType === 1 && 'Case'}
											{lightType === 2 && 'Turbo'}
											{lightType === 3 && 'PlayerLight'}
										</span>
									</div>
									<div className="d-flex w-100 justify-content-between">
										<span className="text-secondary">LEDs:</span>
										<span>{numLedsOnLight}</span>
									</div>
									<div className="d-flex w-100 justify-content-between">
										<span className="text-secondary">Coords:</span>
										<span>
											({xCoord}, {yCoord})
										</span>
									</div>
									<div className="d-flex w-100 justify-content-between">
										<span className="text-secondary">First LED:</span>
										<span>{firstLedIndex}</span>
									</div>
								</div>
							</Popover.Body>
						</Popover>
					}
				>
					<div>
						<LightIcon size={cellWidth} active={active} error={error} />
					</div>
				</OverlayTrigger>
			)}
		</div>
	);
}
