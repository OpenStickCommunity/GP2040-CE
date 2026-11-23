import { forwardRef, useImperativeHandle, useRef } from 'react';
import { Editor, PrismEditor } from 'prism-react-editor';
import { usePrismEditor } from 'prism-react-editor/extensions';
import { useBracketMatcher } from 'prism-react-editor/match-brackets';
import { useHighlightBracketPairs } from 'prism-react-editor/highlight-brackets';
import {
	defaultKeymap,
	useEditHistory,
	useEditorCommands,
} from 'prism-react-editor/commands';
import { insertText } from 'prism-react-editor/utils';
import 'prism-react-editor/prism/languages/json';
import 'prism-react-editor/languages/json';
import 'prism-react-editor/layout.css';

import './JsonEditor.scss';

const Extensions = () => {
	const [editor] = usePrismEditor();
	useBracketMatcher(editor);
	useHighlightBracketPairs(editor);
	useEditorCommands(editor, defaultKeymap);
	useEditHistory(editor);

	return null;
};

export type JsonEditorHandle = {
	// Pretty prints the JSON as a single undoable edit, does nothing if it doesn't parse
	format: () => void;
};

type JsonEditorProps = {
	name: string;
	value: string;
	onChange: (value: string) => void;
	isInvalid?: boolean;
	isValid?: boolean;
};

const JsonEditor = forwardRef<JsonEditorHandle, JsonEditorProps>(
	function JsonEditor({ name, value, onChange, isInvalid, isValid }, ref) {
		const editorRef = useRef<PrismEditor>(null);
		const initialValue = useRef(value).current;
		const latestValue = useRef(value);
		latestValue.current = value;

		useImperativeHandle(ref, () => ({
			format: () => {
				const editor = editorRef.current;
				if (!editor) return;
				let formatted: string;
				try {
					formatted = JSON.stringify(JSON.parse(editor.value), null, 2);
				} catch {
					return;
				}
				if (formatted !== editor.value)
					insertText(editor, formatted, 0, editor.value.length);
			},
		}));

		return (
			<div
				className={`json-editor form-control font-monospace ${isInvalid ? 'is-invalid' : ''} ${isValid ? 'is-valid' : ''}`}
			>
				<Editor
					ref={editorRef}
					language="json"
					value={initialValue}
					lineNumbers={true}
					textareaProps={{ name, 'aria-label': name }}
					onUpdate={(newValue) => {
						if (newValue !== latestValue.current) onChange(newValue);
					}}
				>
					<Extensions />
				</Editor>
			</div>
		);
	},
);

export default JsonEditor;
