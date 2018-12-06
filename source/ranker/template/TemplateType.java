package ranker.template;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public enum TemplateType {

	ALL("all") {
		@Override
		public boolean isValidTemplate(Template template) {
			return true;
		}
	},
	CONSTANTS("constants") {
		@Override
		public boolean isValidTemplate(Template template) {
			return template.containsConstants();
		}
	},
	VARIABLES("variables") {
		@Override
		public boolean isValidTemplate(Template template) {
			return !template.containsConstants();
		}
	};

	public abstract boolean isValidTemplate(Template template);

	private TemplateType(final String name) {
		this.name = name;
	}

	public String getName() {
		return this.name;
	}

	private final String name;

}
