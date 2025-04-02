// This is modeled after Axios to make an easier transition, so this may need more buildout
// if more than simple GET and POST requests are required.

class Http {
	async get(url: string) {
		try {
			const [response] = await Promise.all([fetch(url).then((res) => res.json())]);

			return Promise.resolve({ data: response });
		} catch (err) {
			return Promise.reject(err);
		}
	}

	async post(url: string, body: unknown) {
		try {
			const [response] = await Promise.all([fetch(url, {
				method: 'POST',
				body: JSON.stringify(body)
			}).then((res) => res.json())]);
			return Promise.resolve({ data: response });
		} catch (err) {
			return Promise.reject(err);
		}
	}
}

export default new Http();
