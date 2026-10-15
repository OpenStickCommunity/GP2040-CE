import { useContext } from 'react';
import { useTranslation } from 'react-i18next';
import { AppContext } from '../Contexts/AppContext';
import './Section.scss';

/** @param {{ children?: import('react').ReactNode, title?: import('react').ReactNode, headerRight?: import('react').ReactNode }} props */
const Section = ({ children, title, headerRight = null }) => {
	const { loading } = useContext(AppContext);
	const { t } = useTranslation('');

	return (
		<div className={`card`}>
			<div
				className={`card-header${headerRight ? ' d-flex flex-wrap align-items-center justify-content-between gap-2' : ''}`}
			>
				<strong>{title}</strong>
				{headerRight}
			</div>
			<div className="card-body">
				{loading ? (
					<div className="d-flex justify-content-center align-items-center loading">
						<div className="spinner-border" role="status">
							<span className="visually-hidden">
								{t('Common:loading-text')}
							</span>
						</div>
					</div>
				) : (
					children
				)}
			</div>
		</div>
	);
};

export default Section;
