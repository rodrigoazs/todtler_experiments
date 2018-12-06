package generator.template;

/**
 * @author Jan Van Haaren
 * @version 1.0
 * 
 *          This class represents a literal in a second-order logic formula.
 */
public class TemplateLiteral {

	/**
	 * The truth value of this literal.
	 */
	private boolean positive;

	/**
	 * The number of this literal.
	 */
	private int literalNumber;

	/**
	 * The number of the first argument of this literal.
	 */
	private int argument1Number;

	/**
	 * The number of the second argument of this literal.
	 */
	private int argument2Number;

	/**
	 * Constructs a literal with the given number and argument numbers.
	 * 
	 * @param literalNumber
	 *            The given number.
	 * @param argument1Number
	 *            The given number of the first argument.
	 * @param argument2Number
	 *            The given number of the second argument.
	 */
	public TemplateLiteral(final int literalNumber, final int argument1Number, final int argument2Number) {
		this.positive = true;
		this.setLiteralNumber(literalNumber);
		this.setArgument1Number(argument1Number);
		this.setArgument2Number(argument2Number);
	}

	/**
	 * Constructs a literal by copying the given literal.
	 * 
	 * @param literal
	 *            The given literal.
	 */
	TemplateLiteral(TemplateLiteral literal) {
		this(literal.getLiteralNumber(), literal.getArgument1Number(), literal.getArgument2Number());
		if (!literal.isPositive()) {
			this.negate();
		}
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
	public void negate() {
		this.positive = false;
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

	public int getArgument2Number() {
		return this.argument2Number;
	}

	public void setArgument2Number(final int argument2Number) {
		this.argument2Number = argument2Number;
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

	@Override
	public String toString() {
		String representation = "";
		if (!this.isPositive()) {
			representation += "!";
		}
		representation += "R" + this.getLiteralNumber();
		representation += "(T" + this.getArgument1Number();
		representation += ",T" + this.getArgument2Number() + ")";
		return representation;
	}

}
