package ranker.template;

import java.util.Comparator;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public class TemplateComparator implements Comparator<Template> {

	private static TemplateComparator INSTANCE;

	private TemplateComparator() {
		// NOP
	}

	public static TemplateComparator getInstance() {
		if (INSTANCE == null) {
			INSTANCE = new TemplateComparator();
		}
		return INSTANCE;
	}

	@Override
	public int compare(Template template1, Template template2) {
		if (template1.getScore() < template2.getScore()) {
			return 1;
		}
		else if (template1.getScore() > template2.getScore()) {
			return -1;
		}
		else {
			return 0;
		}
	}

}
