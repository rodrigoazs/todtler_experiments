package generator.template;

import java.util.Comparator;

/**
 * @author Jan Van Haaren
 * @version 1.0
 * 
 *          This class implements a comparator for second-order logic formulas.
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
		if (template1.getLength() < template2.getLength()) {
			return -1;
		}
		else if (template1.getLength() > template2.getLength()) {
			return 1;
		}
		else {
			int comparison = 0;
			for (int i = 0; i < template1.getLength(); i++) {
				comparison = TemplateLiteralComparator.getInstance().compare(template1.getLiterals().get(i), template2.getLiterals().get(i));
				if (comparison != 0) {
					return comparison;
				}
			}
			return 0;
		}
	}

}
