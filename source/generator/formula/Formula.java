package generator.formula;

import generator.Type;
import generator.template.Template;
import generator.template.TemplateLiteral;

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import util.SortedList;

/**
 * @author Jan Van Haaren
 * @version 1.0
 * 
 *          This class represents a first-order logic formula.
 */
public class Formula {

	/**
	 * A reference to the template that this formula is an instance of.
	 */
	private final Template template;

	/**
	 * A list containing the literals appearing in this formula.
	 */
	private final List<FormulaLiteral> literals;

	/**
	 * Constructs a new formula by copying a given formula.
	 * 
	 * @param formula
	 *            The given formula.
	 */
	private Formula(Formula formula) {
		this.template = formula.getTemplate();
		this.literals = new SortedList<FormulaLiteral>(FormulaLiteralComparator.getInstance());
		for (FormulaLiteral literal : formula.getLiterals()) {
			this.addLiteral(new FormulaLiteral(literal));
		}
	}

	/**
	 * Constructs a new formula by instantiating a given template.
	 * 
	 * @param template
	 *            The given template.
	 */
	public Formula(Template template) {
		this.template = template;
		this.literals = new SortedList<FormulaLiteral>(FormulaLiteralComparator.getInstance());
		for (TemplateLiteral literal : template.getLiterals()) {
			this.addLiteral(new FormulaLiteral(literal));
		}
	}

	/**
	 * Returns the referenced template.
	 * 
	 * @return The referenced template.
	 */
	public Template getTemplate() {
		return this.template;
	}

	/**
	 * Returns a list containing the literals appearing in this formula.
	 * 
	 * @return The literals appearing in this formula.
	 */
	public List<FormulaLiteral> getLiterals() {
		return this.literals;
	}

	/**
	 * Adds the given literal to this formula.
	 * 
	 * @param literal
	 *            The given literal.
	 */
	private void addLiteral(FormulaLiteral literal) {
		this.getLiterals().add(new FormulaLiteral(literal));
	}

	/**
	 * Returns the number of literals appearing in this formula.
	 * 
	 * @return The number of literals.
	 */
	public int getLength() {
		return this.getLiterals().size();
	}

	/**
	 * Rearranges the literals appearing in this formula according to <code>FormulaLiteralComparator</code>.
	 */
	public void normalize() {
		Collections.sort(this.getLiterals(), FormulaLiteralComparator.getInstance());
	}

	/**
	 * Verifies if this formula is valid.
	 * 
	 * @return Returns true if this formula is valid, returns false otherwise.
	 */
	public boolean isValid() {
		Map<Integer, Set<String>> map = new HashMap<Integer, Set<String>>();
		for (FormulaLiteral literal : this.getLiterals()) {
			updateMap(map, literal.getArgument1Number(), literal.getArgument1Type());
			updateMap(map, literal.getArgument2Number(), literal.getArgument2Type());
		}
		for (int argumentNumber : map.keySet()) {
			if (map.get(argumentNumber).size() > 1) {
				return false;
			}
		}
		return true;
	}

	/**
	 * Returns a version of this formula with variables replaced by constants in some specified places.
	 * 
	 * @param constantPredicates
	 *            A set of predicates whose variables should be replaced by constants.
	 * @param types
	 *            A map containing the type information of the predicates.
	 * @return Returns a formula with constants instead of variables in some specified places.
	 */
	public Formula getConstantVersion(Set<String> constantPredicates, Map<String, List<Type>> types) {
		boolean constantVersion = false;
		for (FormulaLiteral literal : this.getLiterals()) {
			String literalName = literal.getLiteralName();
			if (constantPredicates.contains(literalName)) {
				constantVersion = constantVersion || true;
			}
		}
		if (constantVersion) {
			Formula constantFormula = new Formula(this);
			for (FormulaLiteral literal : constantFormula.getLiterals()) {
				String literalName = literal.getLiteralName();
				List<Type> typesList = types.get(literalName);
				if (typesList.get(0).canAppearAsConstant()) {
					literal.setArgument1IsConstant(true);
				}
				if (typesList.get(1).canAppearAsConstant()) {
					literal.setArgument2IsConstant(true);
				}
			}
			return constantFormula;
		}
		else {
			return null;
		}
	}

	/**
	 * Verifies if at least one literal contains a constant.
	 * 
	 * @return Returns true if at least one literal contains a constant, returns false otherwise.
	 */
	public boolean isConstantVersion() {
		for (FormulaLiteral literal : this.getLiterals()) {
			if (literal.getArgument1IsConstant() || literal.getArgument2IsConstant()) {
				return true;
			}
		}
		return false;
	}

	@Override
	public boolean equals(Object object) {
		if (Formula.class.isAssignableFrom(object.getClass())) {
			Formula otherFormula = (Formula) object;
			return this.hashCode() == otherFormula.hashCode();
		}
		else {
			return false;
		}
	}

	@Override
	public int hashCode() {
		return this.toString().hashCode();
	}

	public int getIdentifier() {
		int result = 1;
		for (FormulaLiteral literal : this.getLiterals()) {
			result *= literal.getIdentifier();
		}
		return result;
	}

	@Override
	public String toString() {
		String representation = "";
		String prefix = "";
		for (FormulaLiteral literal : this.getLiterals()) {
			representation += prefix;
			representation += literal.toString();
			prefix = " v ";
		}
		return representation;
	}

	private static void updateMap(Map<Integer, Set<String>> map, int argumentNumber, String type) {
		if (!map.containsKey(argumentNumber)) {
			map.put(argumentNumber, new HashSet<String>());
		}
		map.get(argumentNumber).add(type);
	}

}
