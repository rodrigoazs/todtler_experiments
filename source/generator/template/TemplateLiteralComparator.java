package generator.template;

import java.util.Comparator;

/**
 * @author Jan Van Haaren
 * @version 1.0
 * 
 *          This class implements a comparator for literals in second-order logic formulas.
 */
class TemplateLiteralComparator implements Comparator<TemplateLiteral> {

	private static TemplateLiteralComparator INSTANCE;

	private TemplateLiteralComparator() {
		// NOP
	}

	static TemplateLiteralComparator getInstance() {
		if (INSTANCE == null) {
			INSTANCE = new TemplateLiteralComparator();
		}
		return INSTANCE;
	}

	@Override
	public int compare(TemplateLiteral literal1, TemplateLiteral literal2) {
		if (literal1.getLiteralNumber() < literal2.getLiteralNumber()) {
			return -1;
		}
		else if (literal1.getLiteralNumber() > literal2.getLiteralNumber()) {
			return 1;
		}
		else {
			if (literal1.isPositive() && !literal2.isPositive()) {
				return -1;
			}
			else if (!literal1.isPositive() && literal2.isPositive()) {
				return 1;
			}
			else {
				if (literal1.getArgument1Number() < literal2.getArgument1Number()) {
					return -1;
				}
				else if (literal1.getArgument1Number() > literal2.getArgument1Number()) {
					return 1;
				}
				else {
					if (literal1.getArgument2Number() < literal2.getArgument2Number()) {
						return -1;
					}
					else if (literal1.getArgument2Number() > literal2.getArgument2Number()) {
						return 1;
					}
					else {
						return 0;
					}
				}
			}
		}
	}

}
