// This is modeled after Axios to make an easier transition, so this may need more buildout
// if more than simple GET and POST requests are required.

class Http {
	async get(url: string, headers: object = {}) {
		const response = await fetch(url, {
			method: 'GET',
			headers: {
				'Content-Type': 'application/json',
				...headers,
			},
		});

		const json = await response.json();
		return Promise.resolve({ data: json });
	}

	async post(url: string, body: unknown, headers: object = {}) {
		const response = await fetch(url, {
			method: 'POST',
			headers: {
				'Content-Type': 'application/json',
				...headers,
			},
			body: JSON.stringify(body),
		});

		const json = await response.json();
		return Promise.resolve({ data: json });
	}
}

export default new Http();
