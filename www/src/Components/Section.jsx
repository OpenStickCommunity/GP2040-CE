import React, { useContext } from 'react';
import { useTranslation } from 'react-i18next';
import { AppContext } from '../Contexts/AppContext';
import './Section.scss';

const Section = ({ children, title }) => {
	const { loading } = useContext(AppContext);
	const { t } = useTranslation('');

	return (
		<div className={`card`}>
			<div className={`card-header`}>
				<strong>{title}</strong>
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
