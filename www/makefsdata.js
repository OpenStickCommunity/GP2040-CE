import path from 'node:path';
import fs from 'node:fs';

import { fileURLToPath } from 'node:url';

import pako from 'pako';

const __filename = fileURLToPath(import.meta.url);

const root = path.dirname(__filename).replace(path.normalize('www'), '');
const rootwww = path.dirname(__filename);
const buildPath = path.join(rootwww, 'build');

const fsdataPath = path.normalize(path.join(root, 'lib/httpd/fsdata.c'));

// These are the same content types that are used by the original makefsdata
const contentTypes = new Map([
	['html', 'text/html'],
	['htm', 'text/html'],
	[
		'shtml',
		'text/html\r\nExpires: Fri, 10 Apr 2008 14:00:00 GMT\r\nPragma: no-cache',
	],
	[
		'shtm',
		'text/html\r\nExpires: Fri, 10 Apr 2008 14:00:00 GMT\r\nPragma: no-cache',
	],
	[
		'ssi',
		'text/html\r\nExpires: Fri, 10 Apr 2008 14:00:00 GMT\r\nPragma: no-cache',
	],
	['gif', 'image/gif'],
	['png', 'image/png'],
	['jpg', 'image/jpeg'],
	['bmp', 'image/bmp'],
	['ico', 'image/x-icon'],
	['class', 'application/octet-stream'],
	['cls', 'application/octet-stream'],
	['js', 'application/javascript'],
	['ram', 'application/javascript'],
	['css', 'text/css'],
	['swf', 'application/x-shockwave-flash'],
	['xml', 'text/xml'],
	['xsl', 'text/xml'],
	['pdf', 'application/pdf'],
	['json', 'application/json'],
]);

const defaultContentType = 'text/plain';

const shtmlExtensions = new Set(['shtml', 'shtm', 'ssi', 'xml', 'json']);

const skipCompressionExtensions = new Set(['png', 'json']);

const serverHeader = 'GP2040-CE';

const payloadAlignment = 4;
const hexBytesPerLine = 16;

function getFiles(dir) {
	let results = [];
	const list = fs.readdirSync(dir, { withFileTypes: true });
	for (const file of list) {
		file.path = dir + '/' + file.name;
		if (file.isDirectory()) {
			results = results.concat(getFiles(file.path));
		} else if (file.isFile()) {
			results.push(file.path);
		}
	}
	return results;
}

function getLowerCaseFileExtension(file) {
	const ext = file.split('.').pop();
	return ext ? ext.toLowerCase() : '';
}

function fixFilenameForC(filename) {
	let varName = '';
	for (let i = 0; i < filename.length; i++) {
		const c = filename[i];
		if (c >= 'a' && c <= 'z') {
			varName += c;
		} else if (c >= 'A' && c <= 'Z') {
			varName += c;
		} else if (c >= '0' && c <= '9') {
			varName += c;
		} else {
			varName += '_';
		}
	}
	return varName;
}

function CStringLength(str) {
	return Buffer.from(str, 'utf8').length;
}

function createHexString(value, prependComment = false) {
	let column = 0;
	let hexString = '';
	if (typeof value === 'string') {
		const bytes = Buffer.from(value, 'utf8');
		if (prependComment) {
			hexString += `/* "${value}" (${bytes.length} bytes) */\n`;
		}
		bytes.forEach((byte) => {
			hexString += '0x' + byte.toString(16).padStart(2, '0') + ',';
			if (++column >= hexBytesPerLine) {
				hexString += '\n';
				column = 0;
			}
		});
	} else if (Buffer.isBuffer(value) || value instanceof Uint8Array) {
		value.forEach((byte) => {
			hexString += '0x' + byte.toString(16).padStart(2, '0') + ',';
			if (++column >= hexBytesPerLine) {
				hexString += '\n';
				column = 0;
			}
		});
	} else {
		throw new Error('Invalid value type');
	}

	return hexString + '\n';
}

function makefsdata() {
	let fsdata = '';
	fsdata += '#include "fsdata.h"\n';
	fsdata += '\n';
	fsdata += '#define file_NULL (struct fsdata_file *) NULL\n';
	fsdata += '\n';
	fsdata += '#ifndef FS_FILE_FLAGS_HEADER_INCLUDED\n';
	fsdata += '#define FS_FILE_FLAGS_HEADER_INCLUDED 1\n';
	fsdata += '#endif\n';
	fsdata += '#ifndef FS_FILE_FLAGS_HEADER_PERSISTENT\n';
	fsdata += '#define FS_FILE_FLAGS_HEADER_PERSISTENT 0\n';
	fsdata += '#endif\n';
	fsdata += '/* FSDATA_FILE_ALIGNMENT: 0=off, 1=by variable, 2=by include */\n';
	fsdata += '#ifndef FSDATA_FILE_ALIGNMENT\n';
	fsdata += '#define FSDATA_FILE_ALIGNMENT 0\n';
	fsdata += '#endif\n';
	fsdata += '#ifndef FSDATA_ALIGN_PRE\n';
	fsdata += '#define FSDATA_ALIGN_PRE\n';
	fsdata += '#endif\n';
	fsdata += '#ifndef FSDATA_ALIGN_POST\n';
	fsdata += '#define FSDATA_ALIGN_POST\n';
	fsdata += '#endif\n';
	fsdata += '#if FSDATA_FILE_ALIGNMENT==2\n';
	fsdata += '#include "fsdata_alignment.h"\n';
	fsdata += '#endif\n';
	fsdata += '\n';

	let payloadAlignmentDummyCounter = 0;

	const fileInfos = [];

	getFiles(buildPath).forEach((file) => {
		const ext = getLowerCaseFileExtension(file);

		const qualifiedName = '/' + path.relative(buildPath, file).replace(/\\/g, '/');
		const varName = fixFilenameForC(qualifiedName);

		fsdata += '#if FSDATA_FILE_ALIGNMENT==1\n';
		fsdata += `static const unsigned int dummy_align_${varName} = ${payloadAlignmentDummyCounter++};\n`;
		fsdata += '#endif\n';

		const fileContent = fs.readFileSync(file);
		let compressed = fileContent.buffer;
		let isCompressed = false;
		if (!skipCompressionExtensions.has(ext)) {
			compressed = pako.deflate(fileContent, {
				level: 9,
				windowBits: 15,
				memLevel: 9,
			});
			console.log(
				`Compressed ${qualifiedName} from ${fileContent.byteLength} to ${
					compressed.byteLength
				} bytes (${Math.round(
					(compressed.byteLength / fileContent.byteLength) * 100,
				)}%)`,
			);
			if (compressed.byteLength >= fileContent.byteLength) {
				console.log(
					`Skipping compression of ${qualifiedName}, compressed size is larger than original`,
				);
			} else {
				isCompressed = true;
			}
		} else {
			console.log(`Skipping compression of ${qualifiedName} by file extension`);
		}

		const qualifiedNameLength = CStringLength(qualifiedName) + 1;
		const paddedQualifiedNameLength =
			Math.ceil(qualifiedNameLength / payloadAlignment) * payloadAlignment;
		// Zero terminate the qualified name and pad it to the next multiple of payloadAlignment
		const paddedQualifiedName =
			qualifiedName +
			'\0'.repeat(1 + paddedQualifiedNameLength - qualifiedNameLength);
		fsdata += `static const unsigned char data_${varName}[] FSDATA_ALIGN_PRE = {\n`;
		fsdata += `/* ${qualifiedName} (${qualifiedNameLength} chars) */\n`;
		fsdata += createHexString(paddedQualifiedName, false);
		fsdata += '\n';
		fsdata += '/* HTTP header */\n';
		fsdata += createHexString('HTTP/1.0 200 OK\r\n', true);
		fsdata += createHexString(`Server: ${serverHeader}\r\n`, true);
		fsdata += createHexString(
			`Content-Length: ${
				isCompressed ? compressed.byteLength : fileContent.byteLength
			}\r\n`,
			true,
		);
		if (isCompressed) {
			fsdata += createHexString('Content-Encoding: deflate\r\n', true);
		}
		fsdata += createHexString(
			`Content-Type: ${contentTypes.get(ext) ?? defaultContentType}\r\n\r\n`,
			true,
		);
		fsdata += `/* raw file data (${
			isCompressed ? compressed.byteLength : fileContent.byteLength
		} bytes) */\n`;
		fsdata += createHexString(isCompressed ? compressed : fileContent);
		fsdata += '};\n\n';

		fileInfos.push({
			varName,
			paddedQualifiedNameLength,
			isSsiFile: shtmlExtensions.has(ext),
		});
	});

	let prevFile = 'NULL';
	fileInfos.forEach((fileInfo) => {
		fsdata += `const struct fsdata_file file_${fileInfo.varName}[] = {{\n`;
		fsdata += `file_${prevFile},\n`;
		fsdata += `data_${fileInfo.varName},\n`;
		fsdata += `data_${fileInfo.varName} + ${fileInfo.paddedQualifiedNameLength},\n`;
		fsdata += `sizeof(data_${fileInfo.varName}) - ${fileInfo.paddedQualifiedNameLength},\n`;
		fsdata += `FS_FILE_FLAGS_HEADER_INCLUDED | ${
			fileInfo.isSsiFile
				? 'FS_FILE_FLAGS_SSI'
				: 'FS_FILE_FLAGS_HEADER_PERSISTENT'
		}\n`;
		fsdata += '}};\n\n';

		prevFile = fileInfo.varName;
	});

	fsdata += `#define FS_ROOT file_${prevFile}\n`;
	fsdata += `#define FS_NUMFILES ${fileInfos.length}\n`;

	fs.writeFileSync(fsdataPath, fsdata, 'utf8');
}

makefsdata();
