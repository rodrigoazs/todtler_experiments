package ranker;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public enum MergeOperator {

	MULTIPLY("multiply") {
		@Override
		public double merge(double score1, double score2) {
			return score1 * score2;
		}
	},
	MAXIMUM("maximum") {
		@Override
		public double merge(double score1, double score2) {
			return Math.max(score1, score2);
		}
	},
	AVERAGE("average") {
		@Override
		public double merge(double score1, double score2) {
			return (score1 + score2) / 2;
		}
	};

	public abstract double merge(double score1, double score2);

	private MergeOperator(final String name) {
		this.name = name;
	}

	public String getName() {
		return this.name;
	}

	private final String name;

}