import { useEffect, useState } from 'react';
import { DragDropContext, Droppable, Draggable } from 'react-beautiful-dnd';
import './DraggableListGroup.scss';

const reorder = (list, startIndex, endIndex) => {
	const result = Array.from(list);
	const [removed] = result.splice(startIndex, 1);
	result.splice(endIndex, 0, removed);

	return result;
};

const move = (source, destination, droppableSource, droppableDestination) => {
	const sourceClone = Array.from(source);
	const destClone = Array.from(destination);
	const [removed] = sourceClone.splice(droppableSource.index, 1);

	destClone.splice(droppableDestination.index, 0, removed);

	const result = {};
	result[droppableSource.droppableId] = sourceClone;
	result[droppableDestination.droppableId] = destClone;

	return result;
};

const DraggableListGroup = ({
	groupName,
	titles,
	dataSources,
	onChange,
	...props
}) => {
	const [droppableIds, setDroppableIds] = useState([]);
	const [listData, setListData] = useState({});

	useEffect(() => {
		if (onChange)
			onChange(
				Object.keys(listData).reduce((p, n) => {
					p.push(listData[n]);
					return p;
				}, []),
			);
	}, [listData]);

	useEffect(() => {
		setDroppableIds(dataSources.map((v, i) => `${groupName}-${i}`));
		setListData(
			dataSources.reduce(
				(p, n) => ({ ...p, [`${groupName}-${dataSources.indexOf(n)}`]: n }),
				{},
			),
		);
	}, [dataSources, setDroppableIds, setListData]);

	const onDragEnd = (result) => {
		const { source, destination } = result;

		if (!destination) return;

		if (source.droppableId === destination.droppableId) {
			const items = reorder(
				listData[source.droppableId],
				source.index,
				destination.index,
			);

			const newListData = { ...listData };
			newListData[source.droppableId] = items;
			setListData(newListData);
		} else {
			const moved = move(
				listData[source.droppableId],
				listData[destination.droppableId],
				source,
				destination,
			);

			const newListData = { ...listData };
			newListData[source.droppableId] = moved[source.droppableId];
			newListData[destination.droppableId] = moved[destination.droppableId];
			setListData(newListData);
		}
	};

	return (
		<div className="draggable-list-group">
			<DragDropContext onDragEnd={onDragEnd}>
				{droppableIds.map((droppableId, i) => (
					<div key={droppableId} className="draggable-list-container">
						<div className="draggable-list-title">{titles[i]}</div>
						<Droppable key={droppableId} droppableId={droppableId}>
							{(droppableProvided, droppableSnapshot) => (
								<div
									{...droppableProvided.droppableProps}
									ref={droppableProvided.innerRef}
									className={`draggable-list ${
										droppableSnapshot.isDraggingOver
											? 'list-group bg-primary'
											: 'list-group'
									} border border-dark rounded-1`}
								>
									{listData[droppableId].map((item, l) => (
										<Draggable key={item.id} draggableId={item.id} index={l}>
											{(draggableProvided, draggableSnapshot) => (
												<div
													ref={draggableProvided.innerRef}
													{...draggableProvided.draggableProps}
													{...draggableProvided.dragHandleProps}
													className={
														draggableSnapshot.isDragging
															? 'list-group-item active'
															: 'list-group-item'
													}
												>
													{item.label}
												</div>
											)}
										</Draggable>
									))}
									{droppableProvided.placeholder}
								</div>
							)}
						</Droppable>
					</div>
				))}
			</DragDropContext>
		</div>
	);
};

export default DraggableListGroup;
