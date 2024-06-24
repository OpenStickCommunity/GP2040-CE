// This is modeled after Axios to make an easier transition, so this may need more buildout
// if more than simple GET and POST requests are required.

type GetOptions = {
	headers?: Record<string, string>;
	signal?: AbortSignal;
};

class Http {
	async get(url: string, { headers = {}, signal }: GetOptions = {}) {
		try {
			const response = await fetch(url, {
				method: 'GET',
				headers: {
					'Content-Type': 'application/json',
					...headers,
				},
				signal,
			});

			const json = await response.json();
			return Promise.resolve({ data: json });
		} catch (err) {
			return Promise.reject(err);
		}
	}

	async post(url: string, body: unknown, headers: object = {}) {
		try {
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
		} catch (err) {
			return Promise.reject(err);
		}
	}
}

export default new Http();
