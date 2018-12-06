package generator.formula;

import java.util.Comparator;

/**
 * @author Jan Van Haaren
 * @version 1.0
 * 
 *          This class implements a comparator for literals in first-order logic formulas.
 */
class FormulaLiteralComparator implements Comparator<FormulaLiteral> {

	private static FormulaLiteralComparator INSTANCE;

	private FormulaLiteralComparator() {
		// NOP
	}

	static FormulaLiteralComparator getInstance() {
		if (INSTANCE == null) {
			INSTANCE = new FormulaLiteralComparator();
		}
		return INSTANCE;
	}

	@Override
	public int compare(FormulaLiteral literal1, FormulaLiteral literal2) {
		int nameComparison = literal1.getLiteralName().compareTo(literal2.getLiteralName());
		if (nameComparison != 0) {
			return nameComparison;
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
