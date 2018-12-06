package generator.formula;

import generator.template.TemplateLiteral;

/**
 * @author Jan Van Haaren
 * @version 1.0
 * 
 *          This class represents a literal in a first-order logic formula.
 */
public class FormulaLiteral {

	/**
	 * The truth value of this literal.
	 */
	private boolean positive;

	/**
	 * The name of this literal.
	 */
	private String literalName;

	/**
	 * The number of this literal.
	 */
	private int literalNumber;

	/**
	 * The number of the first argument of this literal.
	 */
	private int argument1Number;

	/**
	 * The type of the first argument of this literal.
	 */
	private String argument1Type;

	/**
	 * Whether or not the first argument is a constant.
	 */
	private boolean argument1IsConstant;

	/**
	 * The number of the second argument of this literal.
	 */
	private int argument2Number;

	/**
	 * The type of the second argument of this literal.
	 */
	private String argument2Type;

	/**
	 * Whether or not the second argument is a constant.
	 */
	private boolean argument2IsConstant;

	/**
	 * Constructs a new literal with the given name, argument numbers, and argument types.
	 * 
	 * @param literalName
	 *            The given name.
	 * @param argument1Number
	 *            The given number of the first argument.
	 * @param argument1Type
	 *            The given type of the first argument.
	 * @param argument2Number
	 *            The given number of the second argument.
	 * @param argument2Type
	 *            The given type of the second argument.
	 */
	private FormulaLiteral(final String literalName, final int argument1Number, final String argument1Type, final int argument2Number, final String argument2Type) {
		this(literalName, argument1Number, argument2Number);
		this.setArgument1Type(argument1Type);
		this.setArgument2Type(argument2Type);
	}

	/**
	 * Constructs a new literal with the given name and argument numbers.
	 * 
	 * @param literalName
	 *            The given name.
	 * @param argument1Number
	 *            The given number of the first argument.
	 * @param argument2Number
	 *            The given number of the second argument.
	 */
	private FormulaLiteral(final String literalName, final int argument1Number, final int argument2Number) {
		this.positive = true;
		this.setLiteralName(literalName);
		this.setArgument1Number(argument1Number);
		this.setArgument2Number(argument2Number);
	}

	/**
	 * Constructs a new literal by copying the given literal.
	 * 
	 * @param literal
	 *            The given literal.
	 */
	FormulaLiteral(FormulaLiteral literal) {
		this(literal.getLiteralName(), literal.getArgument1Number(), literal.getArgument1Type(), literal.getArgument2Number(), literal.getArgument2Type());
		if (!literal.isPositive()) {
			this.negate();
		}
		this.setLiteralNumber(literal.getLiteralNumber());
	}

	/**
	 * Constructs a new literal by instantiating the given template.
	 * 
	 * @param literal
	 *            The given template.
	 */
	FormulaLiteral(TemplateLiteral literal) {
		this("", literal.getArgument1Number(), literal.getArgument2Number());
		if (!literal.isPositive()) {
			this.negate();
		}
		this.setLiteralNumber(literal.getLiteralNumber());
	}

	/**
	 * Verifies if this literal is true.
	 * 
	 * @return Returns true if this literal is true, returns false otherwise.
	 */
	public boolean isPositive() {
		return this.positive;
	}

	/**
	 * Negates the truth value of this literal.
	 */
	private void negate() {
		this.positive = false;
	}

	public String getLiteralName() {
		return this.literalName;
	}

	public void setLiteralName(final String literalName) {
		this.literalName = literalName;
	}

	public int getLiteralNumber() {
		return this.literalNumber;
	}

	public void setLiteralNumber(final int literalNumber) {
		this.literalNumber = literalNumber;
	}

	public int getArgument1Number() {
		return this.argument1Number;
	}

	public void setArgument1Number(final int argument1Number) {
		this.argument1Number = argument1Number;
	}

	public String getArgument1Type() {
		return this.argument1Type;
	}

	public void setArgument1Type(final String argument1Type) {
		this.argument1Type = argument1Type;
	}

	public boolean getArgument1IsConstant() {
		return this.argument1IsConstant;
	}

	public void setArgument1IsConstant(boolean value) {
		this.argument1IsConstant = value;
	}

	public int getArgument2Number() {
		return this.argument2Number;
	}

	public void setArgument2Number(final int argument2Number) {
		this.argument2Number = argument2Number;
	}

	public String getArgument2Type() {
		return this.argument2Type;
	}

	public void setArgument2Type(final String argument2Type) {
		this.argument2Type = argument2Type;
	}

	public boolean getArgument2IsConstant() {
		return this.argument2IsConstant;
	}

	public void setArgument2IsConstant(boolean value) {
		this.argument2IsConstant = value;
	}

	@Override
	public boolean equals(Object object) {
		if (TemplateLiteral.class.isAssignableFrom(object.getClass())) {
			TemplateLiteral otherLiteral = (TemplateLiteral) object;
			return this.hashCode() == otherLiteral.hashCode();
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
		return this.toString().replaceAll("\\+", "").hashCode();
	}

	@Override
	public String toString() {
		String representation = "";
		if (!this.isPositive()) {
			representation += "!";
		}
		String constant1 = this.getArgument1IsConstant() ? "+" : "";
		String constant2 = this.getArgument2IsConstant() ? "+" : "";
		representation += this.getLiteralName();
		representation += "(" + constant1 + this.getArgument1Type() + this.getArgument1Number();
		representation += "," + constant2 + this.getArgument2Type() + this.getArgument2Number() + ")";
		return representation;
	}

}
