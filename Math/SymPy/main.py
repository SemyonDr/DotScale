import random

from sympy import Float, Rational


from Tester import GaussTester
from Definitions import GT_Definitions
from GaussFunction import GaussFunction
from GaussFunctionTool import GaussTool
from GaussFunctionApprox import GaussApprox
from SplineCoefs import VSplineCoefsBitDepth
from Statistics import StatItem
from printer import binary_to_str_64bit, binary_64_legend, fxd_to_str

def main():
    # -----------
    # PRODUCING FILES
    # -----------

    if False:
        GaussTool.write_value_approximation_coefs()

    if False:
        GaussTool.write_precalculated_areas()

    if False:
        GaussTool.write_area_poly_coefs()

    # -----------
    # SPLIT EVALUATIONS
    # AND TESTS
    # -----------

    if False:
        GaussTool.evaluate_split(64)

    if False:
        GaussTester.test_areas_for_split(64)


    # -----------
    # VALUE CALCULATIONS
    # AND EVALUATIONS
    # -----------

    if False:
        GaussTool.evaluate_value_calc_steps()

    if False:
        GaussTester.test_value_approx_x60_rng(32)

    if False:
        x_list = [4992733343770550898, 1018867536599930051, 2576014177535941870, 3177047211239085460, 3093322418331663596, 1808126216280093598, 3736522996752759182, 2607656245121771261, 54202879644040372, 3124643868780506712, 7155028603528527259, 6713739399538191658, 205273434943692474, 2986478083223575090, 1777667558431179363, 3580904067180026637, 6581308525333254443, 191790718337500888]
        GaussTester.test_value_approx_x60(x_list)

    # -----------
    # AREA CALCULATIONS
    # AND EVALUATIONS
    # -----------

    if False:
        GaussTool.evaluate_area_poly_value_calc_steps()

    if False:
        GaussTester.test_area_approx_rng_single()

    if False:
        GaussTester.test_area_approx_rng(50)

    if False:
        GaussTester.test_area_subscale_integral_rng(10)

    if False:
        GaussTester.test_area_subscale_integral_rng(10000)

    if False:
        GaussTester.test_area_subscale_integral_x60_single(4066528627030704547, 4071254063142928384, report=True)

    if False:
        intervals = [
            (391998265134951259, 4445867764522350999),
            (4679892017098720169, 7256412861140582561),
            (3484757794108300175, 4498730541098444520),
            (2911093460947251010, 6914916448836742269),
            (2047386320583395361, 5220486656128480941),
            (1356139482228167151, 1978682097498857857),
            (3700365327485564274, 6758338356901996127),
            (6524930503752959270, 7383887308445612295),
            (568217745962552118, 896162684013884411),
            (1513663312995625190, 5748634439185411925),
            (4235312100249258904, 4560738432196708095),
            (1237478082298972808, 3579092729364126108),
            (2650486900090698427, 4099746673247609743),
            (654978673396279716, 3572562154468673967),
            (2433476531800154037, 3282354661901990650),
            (1707645576724566288, 2629881775389690099),
            (412075787708532694, 3970506817725227677),
            (4647358997191779009, 4826663565125639429),
            (1041480481857140437, 3053127932975496566),
            (2890974963659404017, 3154864768265378094)
        ]
        GaussTester.test_area_approx_x60(intervals, False)

    if False:
        GaussTester.test_area_subscale_integral_x60_single(4771625965486762114, 4772821679540207995, report=True)

    if False:
        GaussTester.test_area_approx_x60_single(4066528627030704547, 6809864736875042563)

    if False:
        GaussTester.test_average_value_single_x60(5580614169705092737, 5917912882702873031)

    if True:
        intervals = [
            (99579402609526988, 7425003539903636769),
            (4282528791928049168, 5222276548551959173),
            (4041266619976270482, 5548116211532227319),
            (663946079244624806, 7306549006120607588),
            (173045645342798613, 1540092691748779593),
            (1783715953315677150, 6931468654415414444),
            (2305586127981970496, 6434611647923872009),
            (1263180416376036747, 6565154542137909429),
            (1519294868260867633, 1903655124602195269),
            (1930585818643048505, 4615312712896829529),
            (3585307527296228874, 4287192144782107601),
            (6363420415145810302, 7235853185826607701),
            (2021751247029760359, 4362476304358839033),
            (3235723609660506101, 4085802367987160810),
            (1193917893100952889, 2380335826307277385),
            (4011019120711717403, 6545887153515617785),
            (303290481061047882, 6300248881696507131),
            (4984324640221792316, 5260840561721936979),
            (815386680474915933, 3861970949519987817),
            (2740293431671158567, 3396728004121803012),
            (3136744662242919113, 7461906338596195071),
            (5430701457213243985, 6526758521620832244),
            (485852709621984512, 2721494122332129131),
            (1388857466539258049, 4663871375995740547),
            (4307663339877212826, 5085837385626823366),
            (6686567984966601888, 6712477032642129602),
            (1720558571874756360, 7216269954402423276),
            (3540186200097701853, 4936127041824279502),
            (1349103659731567565, 2640352986970183535),
            (5011943595526582459, 7074419775853882354)
        ]
        GaussTester.test_average_value_x60(intervals)

# -----------
# RUNNING THE CODE
# -----------

# This will ensure that the main code run
# only on original application startup,
# and is not run when this file is loaded
# by derivative processes.
if __name__ == "__main__":
    main()


